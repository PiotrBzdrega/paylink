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
    void stm::sync_worker(std::stop_token stop_token)
    {
        /* Create serial port handler */
        com::serial sync_serial{"/dev/ttyACM1"};

        /* Execute loop until stop requested */
        while (!stop_token.stop_requested())
        {
            std::pair<std::string, std::optional<std::promise<std::string>>> request{};

            /* Wait for new request */
            request_queue.pop(request);

            /* Send message to serial port  */
            auto write_bytes = sync_serial << request.first;

            /* Number of sended bytes is the same as requested message  */
            if (request.first.size() != write_bytes)
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
                sync_serial.reconnect();
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
                /* It is not direct call, but singnals states has change */
                else
                if (signals_changed)
                {
                    pool.submit_task()
                }
            }
        }
    }
    void stm::irq_worker(std::stop_token stop_token)
    {
        com::serial irq_serial{"/dev/ttyACM0"};
        std::string irq;
        auto res = irq_serial >> irq;
        if (res > 0 && irq == IRQ_MSG)
        {
            /* create thread safe queue*/
        }
    }
    std::string stm::create_request(std::string_view request)
    {
        auto prom = std::promise<std::string>{};
        auto fut = prom.get_future();
        request_queue.push_front(std::make_pair(request.data(), prom));

        return fut.get();
    }
    stm::stm(BS::thread_pool<> &pool_) : pool{pool_}
    {
        sync_thr = std::jthread(std::bind_front(&stm::sync_worker, this));
        irq_thr = std::jthread(std::bind_front(&stm::irq_worker, this));
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
    };
}