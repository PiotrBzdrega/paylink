#pragma once
#include <string_view>
#include "Aesimhei.h"

namespace paylink
{
    class dispenser
    {
    private:
        DispenserBlock block;
        std::string_view unitToString();
        std::string_view coinLevelToString();
        std::string_view statusToString();
        bool init_ok{};
    public:
        bool init();
        void debug_info();
        void setInhibit(bool state);
        DispenserBlock *operator&()
        {
            return &block;
        }
    };

    // dispenser::dispenser(/* args */)
    // {
    // }

    // dispenser::~dispenser()
    // {
    // }

}