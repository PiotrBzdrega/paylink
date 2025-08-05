#pragma once
#include <string_view>
#include "Aesimhei.h"

namespace paylink
{

    enum MHE_OPEN_STATE
    {
        SUCCESS = 0,
        ERROR_INVALID_DATA = 1,
        ERROR_BAD_UNIT = 4,
        ERROR_NOT_READY = 3,
        ERROR_GEN_FAILURE = 2,
        ERROR_BUSY = 170,
        ERROR_DEVICE_NOT_CONNECTED = 1167
    };

    class utils
    {
    private:
        /* data */
    public:
        static std::pair<int, std::string_view> LastPayoutStatus();
        static std::pair<int, std::string_view> USBDriverStatus();
        static std::pair<int, std::string_view> PlatformType();
        static std::pair<int, std::string_view> OpenMHEVersion(int InterfaceVersion = INTERFACE_VERSION);
    };
}
