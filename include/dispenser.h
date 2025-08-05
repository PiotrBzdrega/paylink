#pragma once
#include <string_view>
#include "Aesimhei.h"

namespace paylink
{
    class dispenser
    {
    private:
        DispenserBlock block;
    public:
        // dispenser(/* args */);
        // ~dispenser();
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