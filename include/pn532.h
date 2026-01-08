
#include <thread>
#include <nfc/nfc.h>
#include <nfc/nfc-types.h>

namespace nfc
{
    enum class poll_type : uint8_t
    {
        SUCCESS = 0,
        TIMEOUT = -6,
        ERROR = -1
    };


    // Function pointer type for the callback (C-compatible)
    typedef void (*callback)(const char *card_info, int status);

    class pn532
    {
    private:
        nfc_device *pnd{};
        nfc_context *context{};
        void poll_task(std::stop_token stop_token, callback cb, int timeout_sec);

        std::jthread poll_thread;

    public:
        pn532(/* args */);
        int poll(callback cb, int timeout_sec);
        ~pn532();
    };

}