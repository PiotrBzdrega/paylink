/*--------------------------------------------------------------------------*\
 * System Includes.
\*--------------------------------------------------------------------------*/
#include <print>
#include <csignal>
#include <thread>
#include <chrono>
#include <atomic>

/*--------------------------------------------------------------------------*\
 * Custom Includes.
\*--------------------------------------------------------------------------*/
#include "acceptor.h"
#include "dispenser.h"
#include "utils.h"

/*--------------------------------------------------------------------------*\
 * AES Includes.
\*--------------------------------------------------------------------------*/
#include "Aesimhei.h"

/*--------------------------------------------------------------------------*\
 * AbortHandler:
 * -------------
 *
 * This ensures the AESAccess Shared library destructor function is called.
 * If the application exits normaly the destructor is always called, however
 * if the application receives a signal (SIGINT...) the destructor is not
 * called, that is why we need this signal handler (force exit() call).
 *
 * Arguments:
 * ----------
 *
 * Return:
 * -------
 *
\*--------------------------------------------------------------------------*/

using namespace std::chrono_literals;
using namespace paylink;
std::atomic<bool> signal_received(false); // Must be lock-free

void AbortHandler([[maybe_unused]] int signum)
{
    std::println("signal number {}", signum);
    signal_received.store(true, std::memory_order_relaxed); // ✅ Signal-safe
    exit(0);
}

int main()
{

    /*-- Catch signals so we exit cleanly -------------------------------*/
    std::signal(SIGHUP, AbortHandler);
    std::signal(SIGINT, AbortHandler);
    std::signal(SIGQUIT, AbortHandler);
    std::signal(SIGILL, AbortHandler);
    std::signal(SIGTRAP, AbortHandler);
    std::signal(SIGABRT, AbortHandler);
    std::signal(SIGBUS, AbortHandler);
    std::signal(SIGFPE, AbortHandler);
    std::signal(SIGKILL, AbortHandler);
    std::signal(SIGUSR1, AbortHandler);
    std::signal(SIGSEGV, AbortHandler);
    std::signal(SIGUSR2, AbortHandler);
    std::signal(SIGPIPE, AbortHandler);
    std::signal(SIGALRM, AbortHandler);
    std::signal(SIGTERM, AbortHandler);
    std::signal(SIGCHLD, AbortHandler);
    std::signal(SIGCONT, AbortHandler);
    std::signal(SIGSTOP, AbortHandler);
    std::signal(SIGTSTP, AbortHandler);
    std::signal(SIGTTIN, AbortHandler);
    std::signal(SIGTTOU, AbortHandler);
    std::signal(SIGURG, AbortHandler);
    std::signal(SIGXCPU, AbortHandler);
    std::signal(SIGXFSZ, AbortHandler);
    std::signal(SIGVTALRM, AbortHandler);
    std::signal(SIGPROF, AbortHandler);
    std::signal(SIGWINCH, SIG_IGN);
    std::signal(SIGIO, AbortHandler);
    std::signal(SIGPWR, AbortHandler);
    std::signal(SIGSYS, AbortHandler);

    utils::OpenMHEVersion();

    /*-- Determine total amount read ----------------------------------*/
    int LastCurrencyValue = CurrentValue();
    std::println("Initial currency accepted = {} pence", LastCurrencyValue);

    /*-- Determine if there has been an error ---------------------------*/
    auto err_msg = IMHEIConsistencyError(STANDARD_COIN_TIME, STANDARD_NOTE_TIME);
    if (err_msg && *err_msg)
    {
        std::println("[ERROR] {}", err_msg);
        /* TODO: should i return */
    }

    /* The EnableInterface call is used to allow users to enter coins
    or notes into the system. This would be called when a game is
    initialised and ready to accept credit. */
    EnableInterface();

    {
        /*-- Determine total amount paid out --------------------------------*/
        auto amount_paid = ((double)CurrentPaid() / 100.0);
        std::println("Amout paid: {}", amount_paid);
    }

    {
        auto platform_type = utils::PlatformType();
        std::println("Platform type: [{}] {}", platform_type.first, platform_type.second);
    }

    {
        int serial_number = SerialNumber();
        std::println("Serial number: {}", serial_number);
    }

    {
        auto lay_pay = utils::LastPayoutStatus();
        std::println("Last pay status: [{}] {}", lay_pay.first, lay_pay.second);
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
        version = FirmwareVersion(&compile_date[0], &compile_time[0]);
        uint8_t *version_ptr = (uint8_t *)&version;
        std::println("Firmware version: {}.{}.{}.{}", version_ptr[3], version_ptr[2], version_ptr[1], version_ptr[0]);
    }

    {
        auto driver_status = utils::USBDriverStatus();
        std::println("Driver status: [{}] {}", driver_status.first, driver_status.second);
    }

    {
        bool switch7_state = SwitchOpens(7) == SwitchCloses(7);
        bool switch6_state = SwitchOpens(6) == SwitchCloses(6);
        std::println("Switch 6 state: {}", switch6_state == true ? "open" : "closed");
        std::println("Switch 7 state: {}", switch7_state == true ? "open" : "closed");
    }

    while (!signal_received.load(std::memory_order_relaxed))
    /* ACCEPTORS */
    {

        /*
        main
        create_win_acceptors
        on_win_acceptors_show
        DataFn_StartAcceptorTimer - starts timer that calls "DataFn_AcceptorUpdateData" every 500ms
        DataFn_AcceptorUpdateData
        */
        static acceptor Acceptor;
        /* The sequence numbers of the acceptors are contiguous and run from zero upwards. */
        /* The ReadAcceptorDetails call provides a snapshot of all the information
        possessed by the interface on a single unit of money handling equipment. */
        for (size_t i = 0; ReadAcceptorDetails(i, &Acceptor); i++)
        {
            Acceptor.debug_info();
            Acceptor.setInhibit(false);
        }
        std::this_thread::sleep_for(2s);
    }

    /*
    DataFn_SetupDispensers
    DataFn_UpdateDispensers
    */
    /* Dispenser */
    // {
    //     static DispenserBlock Dispenser;
    //     char *DispenserName;
    //     char Buffer[256];

    //     for (int serial = 0; ReadDispenserDetails(serial, &Dispenser); ++serial)
    //     {

    //     }
    // }
    /* The DisableInterface call is used to prevent users from
    entering any more coins or notes. */
    DisableInterface();
    std::println("after DisableInterface");

    return 1;
}