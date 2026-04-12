#include "task.h"

namespace com
{
    void task::specify_future_call()
    {
        next_time_call = std::chrono::floor<std::chrono::milliseconds>(std::chrono::steady_clock::now()) + interval;
    }
    
    bool task::operator()()
    {
        t();
        if (repeat)
        {
            auto val = repeat.value();
            if (val > 0)
            {
                repeat = --val;
            }
            else
            {
                return true;
            }
        }
        specify_future_call();
        return false;
    }
}
