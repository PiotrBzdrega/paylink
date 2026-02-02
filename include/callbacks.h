#pragma once

namespace cb
{
    extern "C"
    {
        typedef void (*BanknoteCallback)(double overall_val, double banknote_val);
        typedef void (*ErrorEventCallback)(const char *msg);
        // typedef void (*SignalChangeCallback)(bool *view, int v_size, int *signals, int s_size);
        // TODO: why do i have there signals ??
        typedef void (*SignalChangeCallback)(bool *view, int v_size);
        typedef void (*CardDetectionCallback)(const char *uid);
        /*
         TODO: consider struct with more info
            timeout happend or error
        */
    }
}
