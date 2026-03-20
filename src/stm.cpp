#include <string>
#include "serial.h"
#include "logger.h"
#include "stm.h"

namespace uc
{
    namespace
    {
        /* MESSAGES */
        constexpr auto GET_SIGNALS_REQ{"GET_SIGNALS"};
        constexpr auto SET_SIGNAL_REQ{"SET_SIGNALS"};
        constexpr auto TEST_REQ{"SET_SIGNALS"};
        constexpr auto IRQ_MSG{"!"};

        /* SERIAL PORTS */

    }
    void stm::sync_worker(std::stop_token stop_token, std::string_view channel_name)
    {
        /* Create serial port handler */
        com::serial sync_serial{channel_name /* "/dev/ttyACM1" */};
        bool connected{false};

        /* Execute loop until stop requested */
        while (!stop_token.stop_requested())
        {
            if (sync_serial.reconnect(connected) == -1)
            {
                connected = false;
                mik::logger::trace("Failed to connect to serial port in sync worker");
                std::this_thread::sleep_for(std::chrono::seconds(10));
                continue;
            }
            else
            {
                connected = true;
            }

            std::pair<std::string, std::optional<std::promise<std::string>>> request{};

            /* Wait for new request */
            request_queue.pop(request);

            /* Send message to serial port  */
            auto write_bytes = sync_serial << request.first;

            /* Number of sended bytes is the same as requested message  */
            if (static_cast<int>(request.first.size()) != write_bytes)
            {
                mik::logger::error("Number of bytes delivered to serial port {} differs from effective request message {}", write_bytes, request.first.size());
            }

            /* Container for response content */
            std::string response;

            /* Read response from serial port */
            auto read_bytes = sync_serial >> response;

            /* error response */
            if (read_bytes <= 0)
            {
                /* Direct calls contains promise that they are waiting for */
                if (request.second.has_value())
                {
                    request.second.value().set_value("");
                }
                connected = false;
                mik::logger::error("Failed to read response from serial port in sync worker");
            }
            else
            {
                bool signals_changed{};

                /* Specific handling for Signals state */
                if (request.first == GET_SIGNALS_REQ)
                {
                    /* States change detection */
                    if (response != last_states)
                    {
                        /* Update current states data */
                        last_states = response;
                        signals_changed = true;
                    }
                }

                /* Direct calls contains promise that they are waiting for */
                if (request.second.has_value())
                {
                    /* Pass value to the waiting instance */
                    request.second.value().set_value(response);
                }
                /* It is not direct call, but signals states has change */
                else if (signals_changed)
                {
                    if (signal_change_callback)
                    {
                        std::array<uint8_t, 3> a{1, 2, 3};
                        // TODO: forward real message not stub
                        pool.detach_task([this, response, a]()
                                         { signal_change_callback(a.data(), 3); });
                    }
                }
            }
        }
    }
    void stm::irq_worker(std::stop_token stop_token, std::string_view channel_name)
    {
        com::serial irq_serial{channel_name /* "/dev/ttyACM0" */};
        bool connected{false};

        /* Execute loop until stop requested */
        while (!stop_token.stop_requested())
        {
            if (irq_serial.reconnect(connected) == -1)
            {
                connected = false;
                mik::logger::trace("Failed to connect to serial port in irq_worker worker");
                std::this_thread::sleep_for(std::chrono::seconds(10));
                continue;
            }
            else
            {
                connected = true;
            }

            std::string irq;
            auto res = irq_serial >> irq;
            if (res > 0)
            {
                if (irq == IRQ_MSG)
                {
                    // TODO: consider obtain some counter from irq
                    /* add request to thread as less critical*/
                    request_queue.emplace_back(GET_SIGNALS_REQ, std::nullopt);
                }
            }
            else
            {
                connected = false;
                mik::logger::error("Failed to read response from serial port in irq worker");
            }
        }
    }

    std::string stm::create_request(std::string_view request)
    {
        auto prom = std::promise<std::string>{};
        auto fut = prom.get_future();
        request_queue.emplace_front(request.data(), std::make_optional(std::move(prom)));

        return fut.get();
    }
    stm::stm(BS::thread_pool<> &pool_) : pool{pool_}
    {
    }
    stm::~stm()
    {
        if (sync_thr.joinable())
        {
            sync_thr.request_stop();
            sync_thr.join();
        }

        if (irq_thr.joinable())
        {
            irq_thr.request_stop();
            irq_thr.join();
        }
        mik::logger::trace("stm destructor end");
    }
    std::string stm::set_signal_req()
    {
        return create_request(SET_SIGNAL_REQ);
    }
    std::string stm::get_signals_req()
    {
        return create_request(GET_SIGNALS_REQ);
    }
    std::string stm::test_req()
    {
        return std::string(TEST_REQ);
    }
    void stm::set_sensors_state_change_callback(cb::SignalChangeCallback func)
    {
        signal_change_callback = func;
    }
    void stm::run_communication(std::string_view sync_channel_name, std::string_view irq_channel_name)
    {
        sync_thr = std::jthread(std::bind_front(&stm::sync_worker, this), sync_channel_name);
        irq_thr = std::jthread(std::bind_front(&stm::irq_worker, this), irq_channel_name);
    };
}