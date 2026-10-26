#pragma once
#include "paylink/typedefs_c_api.h"

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

struct MainSensorChangeCallbackCtx
{
    MainSensorChangeCallback callback{nullptr};
    void *user_data{nullptr};
};

struct DropSensorChangeCallbackCtx
{
    DropSensorChangeCallback callback{nullptr};
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