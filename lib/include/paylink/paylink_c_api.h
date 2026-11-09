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
    // void setLoggerCallback(LoggerCallback func);

    void
    setnewBanknoteCallbackCtx(BanknoteCallback func, void *user_data);

    int
    setCardDetectedCallbackCtx(CardDetectionCallback func, void *user_data);

    void
    setButtonsStateChangeCallbackCtx(ButtonsChangeCallback func, void *user_data);

    void
    setMainSensorChangeCallbackCtx(MainSensorChangeCallback func, void *user_data);

    void
    setDropSensorChangeCallbackCtx(DropSensorChangeCallback func, void *user_data);

    void
    setLoggerCallbackCtx(LoggerCallback func, void *user_data);

    int 
    dispenseCoins(uint32_t amount);

    uint16_t
    getButtonsState();

    bool
    getMainSensor();

    int
    getLastDropSensor();

    void
    setSensorLed(uint16_t number, uint16_t red, uint16_t green, uint16_t blue);

    void
    setAllSensorLed(uint16_t red, uint16_t green, uint16_t blue);

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
     * @brief Gets system information. Returns 0 when the request was performed;
     *        inspect info->result for the device query result. Returns -1 if
     *        the output pointer is null, the system is unavailable, or an error occurs.
     *        The output must be empty or previously released with freeSystemInfo.
     */
    int
    systemInfo(struct SystemInfo *info);

    /**
     * @brief Releases strings owned by SystemInfo. Safe to call with null or repeatedly.
     */
    void freeSystemInfo(struct SystemInfo *info);

    void
    freeString(const char *str);
#ifdef __cplusplus
}
#endif