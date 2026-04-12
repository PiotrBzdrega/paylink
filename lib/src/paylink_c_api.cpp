#include "paylink/paylink_c_api.h"
#include "system.h"
extern "C"
{
    PaylinkHandle createPaylinkSystem(const char *config_path)
    {
        try
        {
            return new paylink::system(config_path);
        }
        catch (const std::exception &e)
        {
            mik::logger::error("Failed to create system: {}", e.what());
            return nullptr;
        }
    }

    void destroyPaylinkSystem(PaylinkHandle handle)
    {
        delete static_cast<paylink::system*>(handle);
    }

    const char *getVersion(PaylinkHandle handle)
    {
        if (handle)
        {
            return static_cast<paylink::system*>(handle)->version().c_str();
        }
        return nullptr;
    }

    void setnewBanknoteCallback(PaylinkHandle handle, BanknoteCallback func)
    {
        if (handle)
        {
            static_cast<paylink::system*>(handle)->set_new_banknote_callback(func);
        }
    }

    int setCardDetectedCallback(PaylinkHandle handle, CardDetectionCallback func)
    {
        if (handle)
        {
            return static_cast<paylink::system*>(handle)->set_card_detected_callback(func);
        }
        return -1;
    }

    void setButtonsStateChangeCallback(PaylinkHandle handle, ButtonsChangeCallback func)
    {
        if (handle)
        {
            static_cast<paylink::system*>(handle)->set_buttons_state_change_callback(func);
        }
    }

    void setSensorsStateChangeCallback(PaylinkHandle handle, SignalChangeCallback func)
    {
        if (handle)
        {
            static_cast<paylink::system*>(handle)->set_sensors_state_change_callback(func);
        }
    }

    // void setErrorEventCallback(PaylinkHandle *handle, ErrorEventCallback func)
    // {
    //     if (handle)
    //     {
    //         static_cast<paylink::system*>(handle)->set_error_event_callback(func);
    //     }
    // }

    int dispenseCoins(PaylinkHandle handle, uint32_t amount)
    {
        if (handle)
        {
            return static_cast<paylink::system*>(handle)->dispense_coins(amount);
        }
        return -1;
    }

    uint16_t getButtonsState(PaylinkHandle handle)
    {
        if (handle)
        {
            return static_cast<paylink::system*>(handle)->get_buttons_state();
        }
        return 0;
    }

    const char *getSensorsState(PaylinkHandle handle)
    {
        if (handle)
        {
            static std::string sensors_state = static_cast<paylink::system*>(handle)->get_sensors_state();
            return sensors_state.c_str();
        }
        return nullptr;
    }

    void setLED(PaylinkHandle handle, int number, bool on, uint32_t interval_ms)
    {
        if (handle)
        {
            static_cast<paylink::system*>(handle)->set_led(number, on, interval_ms);
        }
    }

    void setMotor(PaylinkHandle handle, bool on, uint32_t ms)
    {
        if (handle)
        {
            static_cast<paylink::system*>(handle)->set_motor(on, ms);
        }
    }

    int levelOfCoins(PaylinkHandle handle)
    {
        if (handle)
        {
            return static_cast<paylink::system*>(handle)->level_of_coins();
        }
        return -1;
    }

    int currentCredit(PaylinkHandle handle)
    {
        if (handle)
        {
            return static_cast<paylink::system*>(handle)->current_credit();
        }
        return -1;
    }

}
