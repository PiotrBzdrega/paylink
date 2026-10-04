#pragma once
#include "paylink/typedefs_c_api.h"
#ifdef __cplusplus
extern "C"
{
#endif

    int
    createPaylinkSystem(const char *config_path, LoggerCallback func, void *user_data);

    void
    destroyPaylinkSystem();

    const char *
    getVersion();
    // void setnewBanknoteCallback(BanknoteCallback func);
    // int setCardDetectedCallback(CardDetectionCallback func);
    // void setButtonsStateChangeCallback(ButtonsChangeCallback func);
    // void setSensorsStateChangeCallback(SignalChangeCallback func);
    // void setLoggerCallback(LoggerCallback func);

    void
    setnewBanknoteCallbackCtx(BanknoteCallback func, void *user_data);

    int
    setCardDetectedCallbackCtx(CardDetectionCallback func, void *user_data);

    void
    setButtonsStateChangeCallbackCtx(ButtonsChangeCallback func, void *user_data);

    void
    setSensorsStateChangeCallbackCtx(SignalChangeCallback func, void *user_data);

    void
    setLoggerCallbackCtx(LoggerCallback func, void *user_data);

    int 
    dispenseCoins(uint32_t amount);

    uint16_t
    getButtonsState();

    const char *
    getSensorsState();

    void
    setLED(int number, bool on, uint32_t interval_ms);

    void
    setMotor(bool on, uint32_t ms);

    int
    dispensedCoins();

    const char *
    levelOfCoins();

    int
    currentCredit();

    /**
     * @brief Get the system status (requires more time than single command to obtain the status )
     */
    struct SystemInfo
    systemInfo();

    void
    freeString(const char *str);
#ifdef __cplusplus
}
#endif