#pragma once

#include <thread>
#include <nfc/nfc.h>
#include <nfc/nfc-types.h>
#include "BS_thread_pool.hpp"
#include "callbacks.h"

namespace nfc
{
    class pn532
    {
    private:
        nfc_device *pnd{};
        nfc_context *context{};
        BS::thread_pool<>& pool;
        std::jthread poll_thread;
        void poll_task(std::stop_token stop_token, cb::CardDetectionCallback cb);

    public:
        pn532(BS::thread_pool<>& pool_);
        int poll(cb::CardDetectionCallback cb);
        ~pn532();
    };

}