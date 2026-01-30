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

            need recognize request type to react for it correctly. Do i really need to have it ??

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

            if (read_bytes <= 0)
            {
                /* Direct calls contains promise that they are waiting for */
                if (request.second.has_value())
                {
                    request.second.value().set_value("");
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