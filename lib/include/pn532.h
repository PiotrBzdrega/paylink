#pragma once

#include <thread>
#include <nfc/nfc.h>
#include <nfc/nfc-types.h>
#include "BS_thread_pool.hpp"
// #include "paylink/callbacks.h"
#include "callback_context.h"

namespace nfc
{
    class pn532
    {
    private:
        nfc_device *pnd{};
        nfc_context *context{};
        BS::thread_pool<>& pool;
        std::jthread poll_thread;
        void poll_task(std::stop_token stop_token, CardDetectionCallbackCtx cb_ctx);

    public:
        // not necessary to check tty. It is set manually in /etc/nfc/libnfc.conf "pn532_uart:/dev/ttyUSB0"
        pn532(BS::thread_pool<>& pool_);
        int poll(CardDetectionCallbackCtx cb_ctx);
        ~pn532();
    };

}