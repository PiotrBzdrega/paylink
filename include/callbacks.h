#pragma once

namespace cb
{
    extern "C"
    {
        typedef void (*BanknoteCallback)(int overall_val, int banknote_val);
        typedef void (*ErrorEventCallback)(const char *msg);
        // typedef void (*SignalChangeCallback)(bool *view, int v_size, int *signals, int s_size);
        // TODO: why do i have there signals ??
        typedef void (*SignalChangeCallback)(const uint8_t *view, int v_size);
        typedef void (*ButtonsChangeCallback)(const uint16_t state, const uint16_t open_edge, const uint16_t close_edge);
        typedef void (*CardDetectionCallback)(const char *uid);
        /*
         TODO: consider struct with more info
            timeout happend or error
        */
    }
}
