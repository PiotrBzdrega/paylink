#pragma once
#include "paylink/callbacks.h"

struct BanknoteCallbackCtx
{
    BanknoteCallback callback{nullptr};
    void *user_data{nullptr};
};

// struct ErrorEventCallbackCtx
// {
//     ErrorEventCallback callback;
//     void *user_data;
// };

struct SignalChangeCallbackCtx
{
    SignalChangeCallback callback{nullptr};
    void *user_data{nullptr};
};

struct ButtonsChangeCallbackCtx
{
    ButtonsChangeCallback callback{nullptr};
    void *user_data{nullptr};
};

struct CardDetectionCallbackCtx
{
    CardDetectionCallback callback{nullptr};
    void *user_data{nullptr};
};

// struct LoggerCallbackCtx
// {
//     LoggerCallback callback;
//     void *user_data;
// };