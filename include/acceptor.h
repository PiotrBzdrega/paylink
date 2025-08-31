#pragma once
#include <string_view>
#include "Aesimhei.h"
#include <thread>
#include <chrono>

namespace paylink
{
    class acceptor
    {
    private:
        AcceptorBlock block;
        std::jthread worket_thread;
        std::string_view unitToString();
        std::string_view statusToString();
        void update(std::chrono::milliseconds updateTime);
    public:
        void debug_info();
        void setInhibit(bool state);
        AcceptorBlock *operator&()
        {
            return &block;
        }
        acceptor(std::chrono::milliseconds updateTime);
    };

    // acceptor::acceptor(/* args */)
    // {
    // }

    // acceptor::~acceptor()
    // {
    // }
}
