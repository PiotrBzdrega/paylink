/*--------------------------------------------------------------------------*\
 * System Includes.
\*--------------------------------------------------------------------------*/
// #include <cstdint.h>
// #include <stdlib.h>
// #include <string.h>
// #include <stdbool.h>
#include <print>

/*--------------------------------------------------------------------------*\
 * AES Includes.
\*--------------------------------------------------------------------------*/
#include "Aesimhei.h"

/*--------------------------------------------------------------------------*\
 * main:
 * -----
 *
 *
 * Arguments:
 * ----------
 *
 * Return:
 * -------
 *
\*--------------------------------------------------------------------------*/
int main (int argc, char *argv[])
{
    int OpenStatus = OpenMHE() ;

    /* 
    TODO: make retry
    1 The DLL, application or device areat incompatible revision levels ERROR_INVALID_DATA No retry
    4 General system error ERROR_BAD_UNIT No retry
    3 Paylink has not yet started ERROR_NOT_READY Yes retry
    2 Driver program not running ERROR_GEN_FAILURE Yes retry
    170 The USB link is in use ERROR_BUSY Yes retry
    1167 No Paylink unit is connected ERROR_DEVICE_NOT_CONNECTED Yes retry */
    if (OpenStatus != 0)
    {   
        std::println("IMHEI open failed - {}", OpenStatus);
        return 0 ;
    }

     // Currency acceptance is currently disabled
    int LastCurrencyValue = CurrentValue() ;
    std::println("Initial currency accepted = {} pence",LastCurrencyValue) ;

    /* The EnableInterface call is used to allow users to enter coins
    or notes into the system. This would be called when a game is
    initialised and ready to accept credit. */    
    EnableInterface();

    {
        int mhe_version = OpenMHEVersion(ORIGINAL_VERSION);
        std::println("MHE version: {}",mhe_version);
    }

    {
        int platform_type = PlatformType();
        std::println("Platform type: %d\n",platform_type);
    }

    {
        int serial_number = SerialNumber();
        std::println("Serial number: {}",serial_number);
    }

    {
        int last_pay_status = LastPayStatus();
        std::println("Last pay status: {}",last_pay_status);
    }

    // char Line[80] ;
    // int  i ;
    // int Serial ;

    // if (argc < 2)
    // {   std::println("Enter Serial No: ") ;
    //     if(!fgets(Line, 79, stdin))
    //     {   std::println("Read failed") ;
    //     return 0 ;
    //     }
    // }else
    // {   strcpy(Line, argv[1]) ;
    // }

    // /*-- We need this as BCD hex --------------------------------------*/
    // Serial = 0 ;
    // for (i = 0 ; Line[i] && (Line[i] != '\n') && (Line[i] != '\r'); ++i)
    // {   Serial <<= 4;
    //     Serial |= Line[i] & 0xf ;
    // }

    // if (Serial == 0)
    // {   std::println("Abandoned ({:0>6X})", Serial) ;
    // }else
    // {   SetDeviceKey(2, 40, Serial) ;
    //     std::println("Lumina Serial Number set to {:0>6X}", Serial) ;
    // }

    {
        char compile_date[16], compile_time[16];
        uint32_t version;
        version = FirmwareVersion(&compile_date[0],&compile_time[0]);
        uint8_t* version_ptr = (uint8_t*)&version;
        std::println("Firmware version: {}.{}.{}.{}",version_ptr[3],version_ptr[2],version_ptr[1],version_ptr[0]);
    }

    {
        int driver_status = USBDriverStatus();
        std::println("Driver status: {}",driver_status);
    }


    {
        bool switch7_state = SwitchOpens(7)==SwitchCloses(7);
        bool switch6_state = SwitchOpens(6)==SwitchCloses(6);
        std::println("Switch 6 state: {}",switch6_state==true ? "open" : "closed");
        std::println("Switch 7 state: {}",switch7_state==true ? "open" : "closed");
    }

    /* The DisableInterface call is used to prevent users from
    entering any more coins or notes. */
    DisableInterface();

    return 1 ;
}
