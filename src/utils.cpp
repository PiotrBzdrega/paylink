#include "utils.h"
#include <thread>
#include <print>
#include <chrono>
#include <cstdlib>

using namespace std::chrono_literals;

namespace paylink
{
    std::pair<int, std::string_view> utils::LastPayoutStatus()
    {
        auto value = ::LastPayStatus();
        std::string_view description;
        switch (value)
        {
        case PAY_ONGOING:
            description = "PAY_ONGOING: The interface is in the process of paying out";
            break;
        case PAY_FINISHED:
            description = "PAY_FINISHED: The payout process is up to date";
            break;
        case PAY_EMPTY:
            description = "PAY_EMPTY: The dispenser is empty";
            break;
        case PAY_JAMMED:
            description = "PAY_JAMMED: The dispenser is jammed";
            break;
        case PAY_US:
            description = "PAY_US: Dispenser non functional";
            break;
        case PAY_FRAUD:
            description = "PAY_FRAUD: Dispenser shut down due to fraud attempt";
            break;
        case PAY_FAILED_BLOCKED:
            description = "PAY_FAILED_BLOCKED: The dispenser is blocked";
            break;
        case PAY_NO_HOPPER:
            description = "PAY_NO_HOPPER: No Dispenser matches amount to be paid";
            break;
        case PAY_INHIBITED:
            description = "PAY_INHIBITED: The dispenser is inhibited";
            break;
        case PAY_SECURITY_FAIL:
            description = "PAY_SECURITY_FAIL: The internal self-checks failed";
            break;
        case PAY_HOPPER_RESET:
            description = "PAY_HOPPER_RESET: The hopper reset during a payout";
            break;
        case PAY_NOT_EXACT:
            description = "PAY_NOT_EXACT: The hopper cannot payout the exact amount";
            break;
        case PAY_GHOST:
            description = "PAY_GHOST: This hopper does not really exist";
            break;
        case PAY_NO_KEY:
            description = "PAY_NO_KEY: Waiting on a valid key exchange";
            break;
        default:
            description = " Unknown value";
            break;
        }
        return {value, description};
    }
    std::pair<int, std::string_view> utils::USBDriverStatus()
    {
        /*-- Determine Driver Status ----------------------------------------*/
        auto value = ::USBDriverStatus();
        std::string_view description;
        switch (value)
        {
        case NOT_USB:
            description = "NOT_USB: Interface is to a PCI card";
            break;
        case USB_IDLE:
            description = "USB_IDLE: No driver or other program running";
            break;
        case STANDARD_DRIVER:
            description = "STANDARD_DRIVER: The driver program is running normally";
            break;
        case FLASH_LOADER:
            description = "FLASH_LOADER: The flash re-programming tool is using the link";
            break;
        case MANUFACTURING_TEST:
            description = "MANUFACTURING_TEST: The manufacturing test tool is using the link";
            break;
        case DRIVER_RESTART:
            description = "DRIVER_RESTART: The standard driver is in the process of exiting / restarting";
            break;
        case USB_ERROR:
            description = "USB_ERROR: The driver has received an error from the low level driver";
            break;
        default:
            description = "Driver: Unknown Status";
            break;
        }
        return {value, description};
    }
    std::pair<int, std::string_view> utils::PlatformType()
    {
        auto value = ::PlatformType();
        std::string_view description;
        switch (value)
        {
        case GENOA_UNKNOWN:
            description = "GENOA_UNKNOWN: old versions of the firmware";
            break;
        case GENOA_H8:
            description = "GENOA_H8: The orginal 16 I/O Genoa unit";
            break;
        case GENOA_RX_HID:
            description = "GENOA_RX_HID: The new USB HUB Genoa unit, using the HID interface";
            break;
        case GENOA_RX_FTDI:
            description = "GENOA_RX_FTDI: The new 32 I/O + PWM + Battery Genoa unit, using the same FTDI interface as the H8";
            break;
        case GENOA_LITE_2:
            description = "GENOA_LITE_2: The 4 I/O Lite unit";
            break;
        case GENOA_USB_ONLY:
            description = "GENOA_USB_ONLY: A driver running with a donlge to drive USB units only";
            break;
        case GENOA_USB_MERGED:
            description = "GENOA_USB_MERGED: A driver running with a donlge to drive USB units only";
            break;
        default:
            break;
        }
        return {value, description};
    }
    std::pair<int, std::string_view> utils::OpenMHEVersion(int InterfaceVersion)
    {
        int value;
        std::string_view description;

        while (true)
        {
            bool retry{};
            bool exit{};
            value = ::OpenMHEVersion(InterfaceVersion);

            switch (value)
            {
            case SUCCESS:
                description = "SUCCESS: Money Handling Equipment opened with success";
                break;
            case ERROR_INVALID_DATA:
                description = "ERROR_INVALID_DATA: The DLL, application or device areat incompatible revision levels";
                exit = true;
                break;
            case ERROR_BAD_UNIT:
                description = "ERROR_BAD_UNIT: General system error";
                exit = true;
                break;
            case ERROR_NOT_READY:
                description = "ERROR_NOT_READY: Paylink has not yet started";
                retry = true;
                break;
            case ERROR_GEN_FAILURE:
                description = "ERROR_GEN_FAILURE: Driver program not running";
                retry = true;
                break;
            case ERROR_BUSY:
                description = "ERROR_BUSY: The USB link is in use";
                retry = true;
                break;
            case ERROR_DEVICE_NOT_CONNECTED:
                description = "ERROR_DEVICE_NOT_CONNECTED: No Paylink unit is connected";
                retry = true;
                break;                    
            default:
                break;
            }

            std::println("IMHEI open state - [{}] {} \n{}", value, description,
                         exit ? "exit application" : retry ? "retry OpenMHE after 20s"
                                                           : "");
            if (exit)
            {
                std::exit(EXIT_FAILURE);
            }
            else if (retry)
            {
                std::this_thread::sleep_for(20s);
            }
        };

        return {value,description};
    }
}
