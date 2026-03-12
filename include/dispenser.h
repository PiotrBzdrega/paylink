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
        bool updateBlock();

    public:
        bool setup();
        std::string_view statusToString();
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
        int getLevelOfCoins()
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