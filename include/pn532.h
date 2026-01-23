#pragma once

#include <thread>
#include <nfc/nfc.h>
#include <nfc/nfc-types.h>
#include "BS_thread_pool.hpp"

namespace nfc
{
    extern "C"
    {
        typedef void (*CardDetectionCallback)(const char *uid);
        /*
         TODO: consider struct with more info
            timeout happend or error
        */
    }

    class pn532
    {
    private:
        nfc_device *pnd{};
        nfc_context *context{};
        void poll_task(std::stop_token stop_token, CardDetectionCallback cb, int timeout_sec);
        BS::thread_pool<>& pool;
        std::jthread poll_thread;

    public:
        pn532(BS::thread_pool<>& pool_);
        int poll(CardDetectionCallback cb, int timeout_sec);
        ~pn532();
    };

}