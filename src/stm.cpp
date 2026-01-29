#include "stm.h"
// C library headers
#include <string>

namespace uc
{
    int stm::wait_for_irq()
    {
        std::string irq;
        auto res = irq_serial >> irq;
        if (res > 0 && irq == "!")
        {
            /* create thread safe queue*/
        }
    }
    stm::stm(BS::thread_pool<> &pool_) : pool{pool_} {};
}