/*--------------------------------------------------------------------------*\
 * System Includes.
\*--------------------------------------------------------------------------*/
#include "logger.h"
#include <csignal>
#include <thread>
#include <mutex>
#include <condition_variable>

/*--------------------------------------------------------------------------*\
 * Custom Includes.
\*--------------------------------------------------------------------------*/
#include "system.h"
#include "dispenser.h"

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

std::mutex m;
std::condition_variable cv;
bool return_cond{};

void AbortHandler([[maybe_unused]] int signum)
{
    mik::logger::debug("\nsignal number {}", signum);
    {
        std::lock_guard lk(m);
        return_cond = true;
    }
    cv.notify_one();
}

int main()
{
    mik::logger::setup(false, nullptr, mik::LogLevel::DEBUG);

    /*-- Catch signals so we exit cleanly -------------------------------*/
    std::signal(SIGHUP, AbortHandler);
    std::signal(SIGINT, AbortHandler);
    std::signal(SIGQUIT, AbortHandler);
    std::signal(SIGILL, AbortHandler);
    std::signal(SIGTRAP, AbortHandler);
    std::signal(SIGABRT, AbortHandler);
    std::signal(SIGBUS, AbortHandler);
    std::signal(SIGFPE, AbortHandler);
    // std::signal(SIGKILL, AbortHandler);
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


    paylink::system sys;
    // TODO: IS_NOTE_ACCEPTOR(Unit),



    // {
    //     auto updates = CurrentUpdates();
    //     std::println("updates: {}", updates);
    // }

    mik::logger::trace("before cv.wait");

    {
        std::unique_lock<std::mutex> lk(m);
        cv.wait(
            lk, []
            { return return_cond; });
            mik::logger::debug("cv catched");
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

    return 1;
}