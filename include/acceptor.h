#pragma once
#include <string_view>
#include "Aesimhei.h"

#include <chrono>

namespace paylink
{
    class acceptor
    {
    private:
        AcceptorBlock block;
        std::string_view unitToString();
        std::string_view statusToString();
        void update();
        bool init_ok{};

    public:
        bool init();
        void debug_info();
        void setInhibit(int index, bool state);
        AcceptorBlock *operator&()
        {
            return &block;
        }
        acceptor() = default;
        bool initialized() { return init_ok; }
    };
}
