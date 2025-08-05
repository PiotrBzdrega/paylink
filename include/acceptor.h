#pragma once
#include <string_view>
#include "Aesimhei.h"

namespace paylink
{
    class acceptor
    {
    private:
        AcceptorBlock block;
        std::string_view unitToString();
        std::string_view statusToString();

    public:
        void debug_info();
        void setInhibit(bool state);
        AcceptorBlock *operator&()
        {
            return &block;
        }
    };

    // acceptor::acceptor(/* args */)
    // {
    // }

    // acceptor::~acceptor()
    // {
    // }
}
