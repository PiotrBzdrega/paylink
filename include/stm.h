#include <thread>
#include <utility>
#include <string>
#include <serial.h>
#include "BS_thread_pool.hpp"
#include "thread_safe_queue.h"

namespace uc
{
    enum class CALL_TYPE
    {
        async,
        sync
    };
    class stm
    {
    private:
        com::serial irq_serial{"/dev/ttyACM0"};
        com::serial sync_serial{"/dev/ttyACM1"};
        std::jthread irq_thr;
        std::jthread sync_thr;
        BS::thread_pool<> &pool;
        com::thread_safe_queue<std::pair<CALL_TYPE,std::string>> request_queue;
        int wait_for_irq();
        /*         irq thread adds element to thread save queue and respond with satatus
                sync thread waits for element, when appears -> calls it and execute callback if state has change
                how to iject direct calls to stm with thread safe queue.
                a) insert direct call to queue (in the front [higher priority]) and ask for future that you will be waiting at
                if i received irq when in queue we have already same type request -> drop it
                b) drop thread save queue and use just mutex for each call */
    public:
        stm(BS::thread_pool<> &pool_);
        ~stm() = default;
    };

}