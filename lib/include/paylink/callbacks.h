#pragma once
#include <stdint.h>
#ifdef __cplusplus
extern "C"
{
#endif
    typedef void (*BanknoteCallback)(int overall_val, int banknote_val, void *user_data);
    typedef void (*ErrorEventCallback)(const char *msg, void *user_data);
    // typedef void (*SignalChangeCallback)(bool *view, int v_size, int *signals, int s_size);
    // TODO: why do i have there signals ??
    typedef void (*SignalChangeCallback)(const char *view, void *user_data);
    typedef void (*ButtonsChangeCallback)(const uint16_t state, const uint16_t open_edge, const uint16_t close_edge, void *user_data);
    typedef void (*CardDetectionCallback)(const char *uid, void *user_data);
    typedef void (*LoggerCallback)(const char *msg, void *user_data);

    /*
     TODO: consider struct with more info
        timeout happend or error
    */
#ifdef __cplusplus
}
#endif