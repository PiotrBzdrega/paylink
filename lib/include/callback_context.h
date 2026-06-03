#pragma once
#include "callbacks.h"

struct BanknoteCallbackCtx
{
    BanknoteCallback callback;
    void *user_data;
};

// struct ErrorEventCallbackCtx
// {
//     ErrorEventCallback callback;
//     void *user_data;
// };

struct SignalChangeCallbackCtx
{
    SignalChangeCallback callback;
    void *user_data;
};

struct ButtonsChangeCallbackCtx
{
    ButtonsChangeCallback callback;
    void *user_data;
};

struct CardDetectionCallbackCtx
{
    CardDetectionCallback callback;
    void *user_data;
};

// struct LoggerCallbackCtx
// {
//     LoggerCallback callback;
//     void *user_data;
// };