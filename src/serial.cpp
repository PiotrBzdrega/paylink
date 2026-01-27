// Linux headers
#include <fcntl.h>   // Contains file controls like O_RDWR
#include <errno.h>   // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>  // write(), read(), close()
#include <cstring>   // strerror()
#include "logger.h"
#include "serial.h"

namespace com
{
    int serial::open()
    {
        // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
        fd = ::open(path.data(), O_RDWR);

        // Create new termios struct, we call it 'tty' for convention
        struct termios tty;

        // Read in existing settings, and handle any error
        if (tcgetattr(fd, &tty) != 0)
        {
            mik::logger::error("Error {} from tcgetattr: {}", errno, strerror(errno));
            return -1;
        }

        tty.c_cflag &= ~PARENB;        // Clear parity bit, disabling parity (most common)
        tty.c_cflag &= ~CSTOPB;        // Clear stop field, only one stop bit used in communication (most common)
        tty.c_cflag &= ~CSIZE;         // Clear all bits that set the data size
        tty.c_cflag |= CS8;            // 8 bits per byte (most common)
        tty.c_cflag &= ~CRTSCTS;       // Disable RTS/CTS hardware flow control (most common)
        tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

        tty.c_lflag &= ~ICANON;
        tty.c_lflag &= ~ECHO;                                                        // Disable echo
        tty.c_lflag &= ~ECHOE;                                                       // Disable erasure
        tty.c_lflag &= ~ECHONL;                                                      // Disable new-line echo
        tty.c_lflag &= ~ISIG;                                                        // Disable interpretation of INTR, QUIT and SUSP
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);                                      // Turn off s/w flow ctrl
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); // Disable any special handling of received bytes

        tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
        tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
        // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
        // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

        tty.c_cc[VTIME] = 10; // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
        tty.c_cc[VMIN] = 0;

        // Set in/out baud rate to be 115200
        cfsetispeed(&tty, B115200);
        cfsetospeed(&tty, B115200);

        // Save tty settings, also checking for error
        if (tcsetattr(fd, TCSANOW, &tty) != 0)
        {
            mik::logger::error("Error {} from tcsetattr: {}", errno, strerror(errno));
            return -1;
        }
        return fd; // success
    }

    serial::serial(std::string_view path_) : path(path_)
    {
        open();
    }
    serial::~serial()
    {
        if (fd != -1)
        {
            close(fd);
        }
    }
    int serial::operator<<(std::string_view msg) const
    {
        return write(fd, msg.data(), msg.size());
    }
    int serial::operator>>(std::string &container) const
    {

        // Read bytes. The behaviour of read() (e.g. does it block?,
        // how long does it block for?) depends on the configuration
        // settings above, specifically VMIN and VTIME
        int num_bytes = read(fd, buf.data(), buf.size());

        container.assign(reinterpret_cast<const char*>(buf.data()), num_bytes);

        // n is the number of bytes read. n may be 0 if no bytes were received, and can also be -1 to signal an error.
        if (num_bytes < 0)
        {
            mik::logger::error("Error reading: {}", strerror(errno));
            return -1;
        }

        
        // Here we assume we received ASCII data, but you might be sending raw bytes (in that case, don't try and
        // print it to the screen like this!)
        mik::logger::debug("Read {} bytes. Received message: {}", num_bytes, container);

        return num_bytes;
    }
    std::string serial::list_ports()
    {
        return "https://github.com/wjwwood/serial/blob/main/src/impl/list_ports/list_ports_linux.cc";
    }
}