#include "pn532.h"
#include "logger.h"
#include "version.h"
#include <functional>

namespace nfc
{
    static std::string print_nfc_target(const nfc_target *pnt, bool verbose)
    {
        char *s;
        str_nfc_target(&s, pnt, verbose);
        mik::logger::debug("{}", s);
        std::string str(s);
        nfc_free(s);
        return str;
    }

    void pn532::poll_task(std::stop_token stop_token, CardDetectionCallback cb, int timeout_sec)
    {
        // TODO: integrate timeout_sec
        while (stop_token.stop_requested() == false)
        {
            pnd = nfc_open(context, nullptr);
            if (pnd == nullptr)
            {
                mik::logger::error("Unable to open NFC device.");
                nfc_exit(context);
                exit(EXIT_FAILURE);
            }

            mik::logger::info("NFC reader: {} opened\n", nfc_device_get_name(pnd));

            while (stop_token.stop_requested() == false && pnd != nullptr)
            {
                const uint8_t uiPollNr = 20;
                const uint8_t uiPeriod = 2;
                const nfc_modulation nmModulations[6] = {
                    {.nmt = NMT_ISO14443A, .nbr = NBR_106},
                    {.nmt = NMT_ISO14443B, .nbr = NBR_106},
                    {.nmt = NMT_FELICA, .nbr = NBR_212},
                    {.nmt = NMT_FELICA, .nbr = NBR_424},
                    {.nmt = NMT_JEWEL, .nbr = NBR_106},
                    {.nmt = NMT_ISO14443BICLASS, .nbr = NBR_106},
                };
                const size_t szModulations = 6;

                nfc_target nt;
                int res = 0;

                mik::logger::debug("NFC device will poll during {} ms ({} pollings of {} ms for {} modulations)\n", (unsigned long)uiPollNr * szModulations * uiPeriod * 150, uiPollNr, (unsigned long)uiPeriod * 150, szModulations);
                if ((res = nfc_initiator_poll_target(pnd, nmModulations, szModulations, uiPollNr, uiPeriod, &nt)) < 0)
                {
                    nfc_perror(pnd, "nfc_initiator_poll_target");
                    nfc_close(pnd);
                    pnd = nullptr; // TODO: check if this is needed
                    // nfc_exit(context);
                    // exit(EXIT_FAILURE);
                }

                if (res > 0)
                {
                    auto verbose{true};
                    auto target_info = print_nfc_target(&nt, verbose);
                    printf("Waiting for card removing...");
                    fflush(stdout);
                    while (0 == nfc_initiator_target_is_present(pnd, NULL))
                    {
                    }
                    nfc_perror(pnd, "nfc_initiator_target_is_present");
                    mik::logger::debug("done.\n");
                    // TODO: i think that class related to tasks that has been stored, but not yet started, should be alive longer than this function scope,
                    //  so make sure so is it
                    pool.detach_task([cb, target_info]()
                                     { cb(target_info.data()); });
                }
                else
                {
                    mik::logger::debug("No target found.\n");
                    pool.detach_task([cb]()
                                     { cb(""); });
                }
            }
        }
    }

    pn532::pn532(BS::thread_pool<> &pool_) : pool(pool_)
    {

        // Display libnfc version
        const char *acLibnfcVersion = nfc_version();

        mik::logger::info("{} uses libnfc {}\n", PROJECT_NAME, acLibnfcVersion);

        nfc_init(&context);
        if (context == nullptr)
        {
            mik::logger::error("Unable to init libnfc (malloc)");
            exit(EXIT_FAILURE); // TODO: exception? how to handle this if used as library
        }
    }

    int pn532::poll(CardDetectionCallback cb, int timeout_sec)
    {
                if (poll_thread.joinable()) // check if previous thread ended
        {
            mik::logger::debug("Cannot start polling, previous thread still running");
            return -1; // previous thread still running
        }
        else
        {
            /* jthread implicitly forward stop token to function, bind_front solves it */
            poll_thread = std::jthread(std::bind_front(&pn532::poll_task, this), cb, timeout_sec);
            mik::logger::debug("Polling thread started");
            return 0;
        }
    }

    pn532::~pn532()
    {
        if (pnd)
        {
            // TODO: check what happen when there is no blocking command like poll or init and we call abort
            nfc_abort_command(pnd);
            nfc_close(pnd);
        }

        if (context)
        {
            nfc_exit(context);
        }
    }
}