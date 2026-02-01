#include <thread>
#include <utility>
#include <string>
#include <mutex>
#include <optional>
#include <future>
#include "BS_thread_pool.hpp"
#include "thread_safe_queue.h"

namespace uc
{
    class stm
    {
    private:

        // TODO: signals state from stm should be stored, and compared during async from stm, execute callback if they are different for now use std::string, can be replace with std::array
        std::string signals;
        std::jthread irq_thr;
        std::jthread sync_thr;
        BS::thread_pool<> &pool;
        com::thread_safe_queue<std::pair<std::string, std::optional<std::promise<std::string>>>> request_queue;
        std::string last_states;
        void sync_worker(std::stop_token stop_token);
        void irq_worker(std::stop_token stop_token);
        std::string create_request(std::string_view request);

        /*                 irq thread adds element to thread save queue and respond with satatus
                        sync thread waits for element, when appears -> calls it and execute callback if state has change
                        how to iject direct calls to stm with thread safe queue.
                        a) insert direct call to queue (in the front [higher priority]) and ask for future that you will be waiting at
                        if i received irq when in queue we have already same type request -> drop it
                        b) drop thread save queue and use just mutex for each call */
    public:
        stm(BS::thread_pool<> &pool_);
        ~stm() = default;
        std::string get_signals_req();
        std::string set_signal_req();
        std::string test_req();
    };

}