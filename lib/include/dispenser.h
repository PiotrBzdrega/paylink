#pragma once
#include <string>
#include "Aesimhei.h"
#include <cstdint>
namespace paylink
{
    class dispenser
    {
    private:
        DispenserBlock block;
        std::string_view unitToString();
        std::string coinLevelToString();
        bool updateBlock();

    public:
        bool setup();
        std::string_view statusToString();
        void debug_info();
        void setInhibit(bool state);
        void update()
        {
            updateBlock();
        }
        DispenserBlock *operator&()
        {
            return &block;
        }
        int getDispensedCoins(bool update = true)
        {
            if (update)
            {
                updateBlock();
            }
            return block.Count;
        }
        std::string getLevelOfCoins(bool update = true)
        {
            if (update)
            {
                updateBlock();
            }
            return coinLevelToString();
        }
    };

    // dispenser::dispenser(/* args */)
    // {
    // }

    // dispenser::~dispenser()
    // {
    // }

}