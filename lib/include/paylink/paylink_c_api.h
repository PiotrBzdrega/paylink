#pragma once
#include "paylink/callbacks.h"
#ifdef __cplusplus
extern "C"
{
#endif
    typedef void *PaylinkHandle;

    PaylinkHandle createPaylinkSystem(const char *config_path);
    void destroyPaylinkSystem(PaylinkHandle handle);
    const char *getVersion(PaylinkHandle handle);
    void setnewBanknoteCallback(PaylinkHandle handle, BanknoteCallback func);
    int setCardDetectedCallback(PaylinkHandle handle, CardDetectionCallback func);
    void setButtonsStateChangeCallback(PaylinkHandle handle, ButtonsChangeCallback func);
    void setSensorsStateChangeCallback(PaylinkHandle handle, SignalChangeCallback func);
    int dispenseCoins(PaylinkHandle handle, uint32_t amount);
    uint16_t getButtonsState(PaylinkHandle handle);
    const char *getSensorsState(PaylinkHandle handle);
    void setLED(PaylinkHandle handle, int number, bool on, uint32_t interval_ms);
    void setMotor(PaylinkHandle handle, bool on, uint32_t ms);
    int levelOfCoins(PaylinkHandle handle);
    int currentCredit(PaylinkHandle handle);
#ifdef __cplusplus
}
#endif