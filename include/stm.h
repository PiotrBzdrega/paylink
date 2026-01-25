#include <thread>

namespace uc
{
    class stm
    {
    private:
        std::jthread irq_thread;
    public:
        stm(/* args */);
        ~stm();
    };
    
    stm::stm(/* args */)
    {
    }
    
    stm::~stm()
    {
    }
    
}