#include <thread>

namespace uc
{
    class stm
    {
    private:
        std::jthread irq_thread;
    public:
        stm(/* args */);
        int wait_for_irq();
        ~stm();
    };
    
    stm::stm(/* args */)
    {
    }
    
    stm::~stm()
    {
    }
    
}