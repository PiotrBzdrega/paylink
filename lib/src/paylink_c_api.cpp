#include "paylink/paylink_c_api.h"
#include "system.h"

static std::unique_ptr<paylink::system> handle;

extern "C"
{
    int createPaylinkSystem(const char *config_path)
    {
        if (handle)
        {
            return 0; // System already created
        }
        
        try
        {
            handle = std::make_unique<paylink::system>(config_path);
            return 0; // System created successfully
        }
        catch (const std::exception &e)
        {
            std::println("Failed to create system: {}", e.what());
            return -1;
        }
    }

    void destroyPaylinkSystem()
    {
        handle.reset(); // This will call the destructor of paylink::system
    }

    const char *getVersion()
    {
        if (handle)
        {
            return handle->version().c_str();
        }
        return nullptr;
    }

    void setnewBanknoteCallback(BanknoteCallback func)
    {
        if (handle)
        {
            handle->set_new_banknote_callback(func);
        }
    }

    int setCardDetectedCallback(CardDetectionCallback func)
    {
        if (handle)
        {
            return handle->set_card_detected_callback(func);
        }
        return -1;
    }

    void setButtonsStateChangeCallback(ButtonsChangeCallback func)
    {
        if (handle)
        {
            handle->set_buttons_state_change_callback(func);
        }
    }

    void setSensorsStateChangeCallback(SignalChangeCallback func)
    {
        if (handle)
        {
            handle->set_sensors_state_change_callback(func);
        }
    }

    // void setErrorEventCallback(PaylinkHandle *handle, ErrorEventCallback func)
    // {
    //     if (handle)
    //     {
    //         static_cast<paylink::system*>(handle)->set_error_event_callback(func);
    //     }
    // }

    int dispenseCoins(uint32_t amount)
    {
        if (handle)
        {
            return handle->dispense_coins(amount);
        }
        return -1;
    }

    uint16_t getButtonsState()
    {
        if (handle)
        {
            handle->get_buttons_state();
        }
        return 0;
    }

    const char *getSensorsState()
    {
        if (handle)
        {
            static std::string sensors_state = handle->get_sensors_state();
            return sensors_state.c_str();
        }
        return nullptr;
    }

    void setLED(int number, bool on, uint32_t interval_ms)
    {
        if (handle)
        {
            handle->set_led(number, on, interval_ms);
        }
    }

    void setMotor(bool on, uint32_t ms)
    {
        if (handle)
        {
            handle->set_motor(on, ms);
        }
    }

    int levelOfCoins()
    {
        if (handle)
        {
            return handle->level_of_coins();
        }
        return -1;
    }

    int currentCredit()
    {
        if (handle)
        {
            return handle->current_credit();
        }
        return -1;
    }

}
