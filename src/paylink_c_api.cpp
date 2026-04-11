#include "system.h"
#include "version.h"
extern "C"
{
    paylink::system *createPaylinkSystem(const char *config_path)
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

    void destroyPaylinkSystem(paylink::system *sys)
    {
        delete sys;
    }

    const char *getVersion(paylink::system *sys)
    {
        if (sys)
        {
            return sys->version().c_str();
        }
        return nullptr;
    }

    void setnewBanknoteCallback(paylink::system *sys, cb::BanknoteCallback func)
    {
        if (sys)
        {
            sys->set_new_banknote_callback(func);
        }
    }

    int setCardDetectedCallback(paylink::system *sys, cb::CardDetectionCallback func)
    {
        if (sys)
        {
            return sys->set_card_detected_callback(func);
        }
        return -1;
    }

    void setButtonsStateChangeCallback(paylink::system *sys, cb::ButtonsChangeCallback func)
    {
        if (sys)
        {
            sys->set_buttons_state_change_callback(func);
        }
    }

    void setSensorsStateChangeCallback(paylink::system *sys, cb::SignalChangeCallback func)
    {
        if (sys)
        {
            sys->set_sensors_state_change_callback(func);
        }
    }

    // void setErrorEventCallback(paylink::system *sys, cb::ErrorEventCallback func)
    // {
    //     if (sys)
    //     {
    //         sys->set_error_event_callback(func);
    //     }
    // }

    int dispenseCoins(paylink::system *sys, uint32_t amount)
    {
        if (sys)
        {
            return sys->dispense_coins(amount);
        }
        return -1;
    }

    uint16_t getButtonsState(paylink::system *sys)
    {
        if (sys)
        {
            return sys->get_buttons_state();
        }
        return 0;
    }

    const char *getSensorsState(paylink::system *sys)
    {
        if (sys)
        {
            static std::string sensors_state = sys->get_sensors_state();
            return sensors_state.c_str();
        }
        return nullptr;
    }

    void setLED(paylink::system *sys, int number, bool on, uint32_t interval_ms)
    {
        if (sys)
        {
            sys->set_led(number, on, interval_ms);
        }
    }

    void setMotor(paylink::system *sys, bool on, uint32_t ms)
    {
        if (sys)
        {
            sys->set_motor(on, ms);
        }
    }

    int levelOfCoins(paylink::system *sys)
    {
        if (sys)
        {
            return sys->level_of_coins();
        }
        return -1;
    }

    int currentCredit(paylink::system *sys)
    {
        if (sys)
        {
            return sys->current_credit();
        }
        return -1;
    }

}
