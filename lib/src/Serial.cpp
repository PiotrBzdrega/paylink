#include "Serial.h"
#include "logger.h"
#include <format>
#include <optional>
#include <string.h> //strerror
#include <unistd.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <linux/serial.h>

namespace com
{
    namespace
    {

        timespec
        timespec_from_ms(const uint32_t millis)
        {
            timespec time;
            time.tv_sec = millis / 1e3;
            time.tv_nsec = (millis - (time.tv_sec * 1e3)) * 1e6;
            return time;
        }
    }

    expect<bool> Serial::get(int modem_line)
    {
        if (tty_fd_ == -1)
        {
            return std::unexpected("Serial port not open");
        }

        unsigned int arg;

        /* Get modem status lines */
        if (ioctl(tty_fd_, TIOCMGET, &arg) < 0)
        {
            return std::unexpected(std::format("TIOCMGET: Failed to get states of modem lines; errno, strerror(errno): {} {}", errno, strerror(errno)));
        }
        return !!(arg & modem_line);
    }

    expect<void> Serial::wait_for(int modem_line, bool expected_state, uint32_t interval_ms, uint32_t timeout_ms)
    {
        auto rem_timeout_ms = timeout_ms;
        while (true)
        {
            if (auto status = get(modem_line); !status)
            {
                return std::unexpected(status.error());
            }
            else if (status.value() == expected_state)
            {
                break;
            }
            else
            {
                usleep(interval_ms * 1000);
                rem_timeout_ms -= interval_ms;

                if (rem_timeout_ms <= interval_ms)
                {
                    return std::unexpected(std::format("Timeout waiting for {}", modem_line));
                }
            }
        }
        return expect<void>();
    }

    Serial::Serial(std::string_view port /* , int baudrate */) : port_(port),
                                                                 event_fd_(eventfd(0, 0))

    {
    }
    Serial::~Serial()
    {
        if (tty_fd_ != -1)
        {
            ::close(tty_fd_);
        }
        if (event_fd_ != -1)
        {
            ::close(event_fd_);
        }
    }
    expect<void> Serial::open()
    {
        tty_fd_ = ::open(port_.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (tty_fd_ == -1)
        {
            return std::unexpected("Failed to open serial port :" + port_);
        }
        return configure();
    }

    expect<ReconnectResult> Serial::reconnect(bool keep_valid_fd)
    {
        if (tty_fd_ != -1 && !keep_valid_fd)
        {
            ::close(tty_fd_);
            tty_fd_ = -1;
        }
        else if (tty_fd_ != -1 && keep_valid_fd)
        {
            // mik::logger::trace("Serial port already open with fd {}, skipping reconnect", tty_fd_);
            return ReconnectResult::AlreadyOpen;
        }

        /* return 0 if successful, -1 if failed */
        if (auto res = open(); !res)
        {
            return std::unexpected(res.error());
        }
        return ReconnectResult::Success;
    }

    expect<void> Serial::configure()
    {
        struct termios options; // The options for the file descriptor
        if (tcgetattr(tty_fd_, &options) != 0)
        {
            auto msg = std::format("Failed to get terminal attributes; errno, strerror(errno): {} {}", errno, strerror(errno));
            return std::unexpected(msg);
        }
        // Configure the terminal attributes here
        // set up raw mode / no echo / binary
        options.c_cflag |= (tcflag_t)(CLOCAL | CREAD);
        options.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL |
                                        ISIG | IEXTEN); //|ECHOPRT

        options.c_oflag &= (tcflag_t) ~(OPOST);
        options.c_iflag &= (tcflag_t) ~(INLCR | IGNCR | ICRNL | IGNBRK);
#ifdef IUCLC
        options.c_iflag &= (tcflag_t)~IUCLC;
#endif
#ifdef PARMRK
        options.c_iflag &= (tcflag_t)~PARMRK;
#endif

        ::cfsetispeed(&options, B115200);
        ::cfsetospeed(&options, B115200);

        // setup char len
        options.c_cflag &= (tcflag_t)~CSIZE;
        options.c_cflag |= CS8;

        // setup stopbits
        options.c_cflag &= (tcflag_t) ~(CSTOPB);

        // setup parity
        options.c_iflag &= (tcflag_t) ~(INPCK | ISTRIP);
        options.c_cflag &= (tcflag_t) ~(PARENB | PARODD); // none

        // setup flow control
        options.c_iflag &= (tcflag_t) ~(IXON | IXOFF | IXANY); // xon/xoff false
        options.c_cflag &= (unsigned long)~(CRTSCTS);          // rtscts false

        // http://www.unixwiz.net/techtips/termios-vmin-vtime.html
        // this basically sets the read call up to be a polling read,
        // but we are using select to ensure there is data available
        // to read before each call, so we should never needlessly poll
        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = 0;

        // activate settings
        if (tcsetattr(tty_fd_, TCSANOW, &options) != 0)
        {
            auto msg = std::format("Failed to set terminal attributes; errno, strerror(errno): {} {}", errno, strerror(errno));
            return std::unexpected(msg);
        }
        return {};
    }

    expect<std::string_view> Serial::read(std::move_only_function<bool(std::string_view)> pred, uint32_t timeout_ms)
    {
        if (tty_fd_ == -1)
        {
            return std::unexpected("Serial port not open");
        }

        int bytes_readed{};
        /* Initially without timeout */
        std::optional<timespec> timeout_val{};

        for (;;)
        {
            // Setup a select call to block for serial data
            fd_set readfds;
            FD_ZERO(&readfds);
            FD_SET(tty_fd_, &readfds);
            FD_SET(event_fd_, &readfds);

            int res = pselect((tty_fd_ > event_fd_ ? tty_fd_ : event_fd_) + 1, &readfds, nullptr, nullptr, timeout_val.has_value() ? &timeout_val.value() : nullptr, nullptr);

            /* Error */
            if (res < 0)
            {
                // Select was interrupted
                if (errno == EINTR)
                {
                    mik::logger::debug("Select was interrupted by signal, retrying...");
                    continue;
                }
                else
                {
                    mik::logger::error("Select failed with error: {}:{}", strerror(errno), errno);
                    // Otherwise there was some error
                    return std::unexpected(std::format("{}:{}", strerror(errno), errno));
                }
            }

            /* Timeout occurred */
            if (res == 0)
            {
                mik::logger::error("Select timed out after {} ms with {} bytes read", timeout_ms, bytes_readed);
                return std::unexpected(std::format("Timeout! with {} bytes received", bytes_readed));
            }

            if (FD_ISSET(event_fd_, &readfds))
            {
                mik::logger::debug("Received notification to interrupt blocking select()!");
                return std::unexpected("notification to interrupt blocking select()!");
            }

            if (FD_ISSET(tty_fd_, &readfds))
            {
                int read_res = ::read(tty_fd_, r_buffer_.data() + bytes_readed, r_buffer_.size() - bytes_readed);
                /* Error */
                if (read_res < 0)
                {
                    if (errno == EINTR /* || errno == EAGAIN || errno == EWOULDBLOCK */)
                    {
                        continue;
                    }
                    else
                    {
                        mik::logger::error("read {}:{}", strerror(errno), errno);
                        return std::unexpected(std::format("read {}:{}", strerror(errno), errno));
                    }
                }
                /*  TODO:?? */
                if (read_res == 0)
                {
                    mik::logger::error("No data read from serial port, but select indicated data is available!");
                    return std::unexpected("No data read from serial port, but select indicated data is available!");
                }

                bytes_readed += read_res;

                if (pred(std::string_view(r_buffer_.data(), bytes_readed)))
                {
                    return std::string_view(r_buffer_.data(), bytes_readed);
                }
                else
                {
                    mik::logger::debug("Predicate not satisfied, continuing to read more data, bytes_readed: {}", bytes_readed);
                    if (bytes_readed == static_cast<int>(r_buffer_.size()))
                    {
                        mik::logger::error("Cannot read more data, buffer is full but predicate is not satisfied!");
                        return std::unexpected("Cannot read more data, buffer is full but predicate is not satisfied!");
                    }

                    timeout_val = timespec_from_ms(timeout_ms);
                    continue;
                }
            }

            /* Fallback -> should not happen */
            return std::unexpected("Select returned > 0 but no fd is set!");
        }
    }
    expect<int> Serial::write(std::string_view data, uint32_t timeout_ms)
    {
        if (tty_fd_ == -1)
        {
            return std::unexpected("Serial port not open");
        }
        int bytes_written{};
        /* Initially without timeout */
        timespec timeout_val(timespec_from_ms(timeout_ms));
        fd_set writefds;
        FD_ZERO(&writefds);
        FD_SET(tty_fd_, &writefds);

        for (;;)
        {
            int res = pselect(tty_fd_ + 1, nullptr, &writefds, nullptr, &timeout_val, nullptr);

            /* Error */
            if (res < 0)
            {
                // Select was interrupted
                if (errno == EINTR)
                {
                    continue;
                }
                else
                {
                    // Otherwise there was some error
                    return std::unexpected(std::format("{}:{}", strerror(errno), errno));
                }
            }

            /* Timeout occurred */
            if (res == 0)
            {
                return std::unexpected(std::format("Timeout! with {} bytes written", bytes_written));
            }

            if (FD_ISSET(tty_fd_, &writefds))
            {
                /* write to port */
                int write_res = ::write(tty_fd_, data.data() + bytes_written, data.size() - bytes_written);

                /* Error */
                if (write_res < 0)
                {
                    if (errno == EINTR /* || errno == EAGAIN || errno == EWOULDBLOCK */)
                    {
                        continue;
                    }
                    else
                    {
                        return std::unexpected(std::format("write {}:{}", strerror(errno), errno));
                    }
                }
                /*  TODO:?? */
                if (write_res == 0)
                {
                    return std::unexpected("No data written to serial port, but select indicated data is available!");
                }

                bytes_written += write_res;
                if (bytes_written == static_cast<int>(data.size()))
                {
                    return bytes_written;
                }
                else if (bytes_written < static_cast<int>(data.size()))
                {
                    continue;
                }
                else
                {
                    return std::unexpected("More bytes written than expected!");
                }
            }
        }
    }

    expect<NotificationType> Serial::wait_for_notification()
    {
        if (tty_fd_ == -1)
        {
            mik::logger::error("Serial port not open, cannot wait for notification");
            return std::unexpected("Serial port not open");
        }

        unsigned int arg;
        /* Get modem status lines */
        if (ioctl(tty_fd_, TIOCMGET, &arg) < 0)
        {
            mik::logger::error("TIOCMGET: Failed to get states of modem lines; errno, strerror(errno): {} {}", errno, strerror(errno));
            return std::unexpected(std::format("TIOCMGET: Failed to get states of modem lines; errno, strerror(errno): {} {}", errno, strerror(errno)));
        }

        if (arg & TIOCM_DSR)
        {
            mik::logger::debug("DSR is high!");
            return NotificationType::DSR;
        }

        if (arg & TIOCM_RI)
        {
            mik::logger::debug("RI is high!");
            return NotificationType::RI;
        }

        // TODO: store previous state and check if it changed, otherwise wait for change
        // if (arg & TIOCM_CD)
        // {
        //     return NotificationType::DCD;
        // }

        struct serial_icounter_struct before, after;

        /* Get initial state */
        if (ioctl(tty_fd_, TIOCGICOUNT, &before) < 0)
        {
            return std::unexpected(std::format("TIOCGICOUNT: Failed to get serial counter values; errno, strerror(errno): {} {}", errno, strerror(errno)));
        }

        mik::logger::debug("Before:  CTS={}, DSR={}, RNG={}, DCD={}", before.cts, before.dsr, before.rng, before.dcd);
        /* Data Set Ready  - drop sensor change detected, Data Carrier Detect - main sensor change, Ring Indicator - We decided to stop TIOCMIWAIT */
        if (ioctl(tty_fd_, TIOCMIWAIT, TIOCM_DSR | TIOCM_CD | TIOCM_RI) < 0)
        {
            mik::logger::error("TIOCMIWAIT: Failed in waiting for notification; errno, strerror(errno): {} {}", errno, strerror(errno));
            return std::unexpected(std::format("TIOCMIWAIT: Failed in waiting for notification; errno, strerror(errno): {} {}", errno, strerror(errno)));
        }

        /* Get modem status lines */
        if (ioctl(tty_fd_, TIOCMGET, &arg) < 0)
        {
            mik::logger::error("TIOCMGET: Failed to get states of modem lines; errno, strerror(errno): {} {}", errno, strerror(errno));
            return std::unexpected(std::format("TIOCMGET: Failed to get states of modem lines; errno, strerror(errno): {} {}", errno, strerror(errno)));
        }

        if (arg & TIOCM_DSR)
        {
            mik::logger::debug("DSR is high!");
            return NotificationType::DSR;
        }

        if (arg & TIOCM_RI)
        {
            mik::logger::debug("RI is high!");
            return NotificationType::RI;
        }

        /* Get state after notification */
        if (ioctl(tty_fd_, TIOCGICOUNT, &after) < 0)
        {
            mik::logger::error("TIOCGICOUNT: Failed to get serial counter values after notification; errno, strerror(errno): {} {}", errno, strerror(errno));
            return std::unexpected(std::format("TIOCGICOUNT: Failed to get serial counter values after notification; errno, strerror(errno): {} {}", errno, strerror(errno)));
        }
        // if (after.dsr != before.dsr)
        // {
        //     mik::logger::debug("DSR changed! (Count: {} -> {})", before.cts, after.cts);
        //     return std::unexpected("DSR changed!");
        // }
        if (after.dcd != before.dcd)
        {
            mik::logger::debug("DCD changed! (Count: {} -> {})", before.dcd, after.dcd);
            return NotificationType::DCD;
        }
        // else if (after.rng != before.rng)
        // {
        //     mik::logger::debug("RI changed! (Count: {} -> {})", before.rng, after.rng);
        //     return {};
        // }

        mik::logger::error("Unexpected change detected! After:  CTS={}/{}, DSR={}/{}, RNG={}/{}, DCD={}/{}", after.cts, !!(arg & TIOCM_CTS), after.dsr, !!(arg & TIOCM_DSR), after.rng, !!(arg & TIOCM_RI), after.dcd, !!(arg & TIOCM_CD));
        return std::unexpected(std::format("Unexpected change detected! After:  CTS={}/{}, DSR={}/{}, RNG={}/{}, DCD={}/{}\n", after.cts, !!(arg & TIOCM_CTS), after.dsr, !!(arg & TIOCM_DSR), after.rng, !!(arg & TIOCM_RI), after.dcd, !!(arg & TIOCM_CD)));
    }

    expect<void> Serial::unlock_wait_notification(std::string_view data)
    {
        if (tty_fd_ == -1)
        {
            mik::logger::error("Serial port not open, cannot unlock wait for notification");
            return std::unexpected("Serial port not open");
        }
        /* Write wakeup data */
        if (auto res = write(data); !res)
        {
            mik::logger::error("Failed to write wakeup data to serial port: {}", res.error());
            return std::unexpected(res.error());
        }
        else
        {
            mik::logger::debug("Wrote wakeup data to serial port: {} bytes", res.value());
            return {};
        }
    }

    expect<bool> Serial::get_dcd()
    {
        return get(TIOCM_CAR);
    }

    expect<bool> Serial::get_dsr()
    {
        return get(TIOCM_DSR);
    }

    expect<bool> Serial::get_ri()
    {
        return get(TIOCM_RI);
    }

    expect<void> Serial::wait_for_low_dsr(uint32_t interval_ms, uint32_t timeout_ms)
    {
        return wait_for(TIOCM_DSR, false, interval_ms, timeout_ms);
    }

    expect<void> Serial::wait_for_low_ri(uint32_t interval_ms, uint32_t timeout_ms)
    {
        return wait_for(TIOCM_RI, false, interval_ms, timeout_ms);
    }

    void Serial::interrupt_wait()
    {
        uint64_t val{1};
        ::write(event_fd_, &val, sizeof(val));
    }
}