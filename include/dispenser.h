#pragma once
#include <string_view>
#include "Aesimhei.h"
#include <cstdint>
namespace paylink
{
    class dispenser
    {
    private:
        DispenserBlock block;
        std::string_view unitToString();
        std::string_view coinLevelToString();
        std::string_view statusToString();
        bool updateBlock();

    public:
        bool setup();
        void debug_info();
        void setInhibit(bool state);
        DispenserBlock *operator&()
        {
            return &block;
        }
        int getDispensedCoins()
        {
            updateBlock();
            return block.Count;
        }
        uint32_t getLevelOfCoins()
        {
            updateBlock();
            return block.Value;
        }
    };

    // dispenser::dispenser(/* args */)
    // {
    // }

    // dispenser::~dispenser()
    // {
    // }

}