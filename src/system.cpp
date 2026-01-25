#include "system.h"
#include "utils.h"
#include "ImheiEvent.h"
#include "logger.h"
#include "version.h"
#include <chrono>
#include <functional>

using namespace std::chrono_literals;

namespace
{
    std::string_view decode_event(int event)
    {
        switch (event)
        {
        case IMHEI_NULL:
            return "Null";
        case IMHEI_INTERFACE_START:
            return "Interface Start";
        case IMHEI_APPLICATION_START:
            return "Application Start";
        case IMHEI_APPLICATION_EXIT:
            return "Application Exit";
        case IMHEI_OVERFLOW:
            return "Overflow";

        /*-- Coin Acceptors -----------------------------------------------*/
        case IMHEI_COIN_NOW_OK:
            return "Coin: Now Ok";
        case IMHEI_COIN_UNIT_REPORTED_FAULT:
            return "Coin: Unit Reported Fault";
        case IMHEI_COIN_UNIT_TIMEOUT:
            return "Coin: Unit Timeout";
        case IMHEI_COIN_UNIT_RESET:
            return "Coin: Unit Reset";
        case IMHEI_COIN_SELF_TEST_REFUSED:
            return "Coin: Self Test Refused";
        case IMHEI_COIN_REJECT_COIN:
            return "Coin: Reject Coin";
        case IMHEI_COIN_INHIBITED_COIN:
            return "Coin: Inhibited Coin";
        case IMHEI_COIN_FRAUD_ATTEMPT:
            return "Coin: Fraud Attempt";
        case IMHEI_COIN_ACCEPTOR_JAM:
            return "Coin: Acceptor Jam";
        case IMHEI_COIN_COIN_RETURN:
            return "Coin: Coin Return";
        case IMHEI_COIN_SORTER_JAM:
            return "Coin: Sorter Jam";
        case IMHEI_COIN_INTERNAL_PROBLEM:
            return "Coin: Internal Problem";
        case IMHEI_COIN_UNCLASSIFIED_EVENT:
            return "Coin: Unclassified Event";

        /*-- Note Acceptors -----------------------------------------------*/
        case IMHEI_NOTE_NOW_OK:
            return "Note: Now Ok";
        case IMHEI_NOTE_UNIT_REPORTED_FAULT:
            return "Note: Unit Reported Fault";
        case IMHEI_NOTE_UNIT_TIMEOUT:
            return "Note: Unit Timeout";
        case IMHEI_NOTE_UNIT_RESET:
            return "Note: Unit Reset";
        case IMHEI_NOTE_SELF_TEST_REFUSED:
            return "Note: Self Test Refused";
        case IMHEI_NOTE_REJECT_NOTE:
            return "Note: Reject Note";
        case IMHEI_NOTE_INHIBITED_NOTE:
            return "Note: Inhibited Note";
        case IMHEI_NOTE_NOTE_MISREAD:
            return "Note: Note Misread";
        case IMHEI_NOTE_FRAUD_ATTEMPT:
            return "Note: Fraud Attempt";
        case IMHEI_NOTE_ACCEPTOR_JAM:
            return "Note: Acceptor Jam";
        case IMHEI_NOTE_ACCEPTOR_JAM_FIXED:
            return "Note: Acceptor Jam Fixed";
        case IMHEI_NOTE_NOTE_RETURNED:
            return "Note: Note Returned";
        case IMHEI_NOTE_STACKER_PROBLEM:
            return "Note: Stacker Problem";
        case IMHEI_NOTE_STACKER_FIXED:
            return "Note: Stacker Fixed";
        case IMHEI_NOTE_INTERNAL_ERROR:
            return "Note: Internal Error";
        case IMHEI_NOTE_UNCLASSIFIED_EVENT:
            return "Note: Unclassified Event";

        /*-- Coin Dispenser ------------------------------------------------*/
        case IMHEI_COIN_DISPENSER_NOW_OK:
            return "Hopper: Now OK";
        case IMHEI_COIN_DISPENSER_REPORTED_FAULT:
            return "Hopper: Unit Reported Fault";
        case IMHEI_COIN_DISPENSER_TIMEOUT:
            return "Hopper: Unit Timeout";
        case IMHEI_COIN_DISPENSER_RESET:
            return "Hopper: Unit Reset";
        case IMHEI_COIN_DISPENSER_SELF_TEST_REFUSED:
            return "Hopper: Self Test Refused";
        case IMHEI_COIN_DISPENSER_FRAUD_ATTEMPT:
            return "Hopper: Fraud Attempt";
        case IMHEI_COIN_DISPENSER_UPDATE:
            return "Hopper: Level Updated";

        default:
            return "Unknown Message Code";
        }
    }
}

namespace paylink
{
    system::system() : nfc_reader(pool)
    {
        if (init())
        {
            note_acceptor.init();

            if (coin_dispenser.setup() == false)
            {
                // TODO: how to handle paylink DisableInterface if exception thrown
                throw std::runtime_error("Dispenser setup failed");
            }

            worker_thread = std::jthread(std::bind_front(&system::update_data, this), 500ms);
        }
    }

    void system::set_banknote_callback(BanknoteCallback func)
    {
        banknote_callback = func;
    }

    bool system::init()
    {
        if (auto state = utils::OpenMHEVersion(); state.first != SUCCESS)
        {
            return false;
        }

        /*-- Determine total amount read ----------------------------------*/
        /*
            This value should be read following the call to OpenMHE and before the call to
            EnableInterface to establish a starting point before any coins or notes are read
        */
        StartTotalAmountRead = TotalAmountRead = CurrentValue();
        mik::logger::debug("Initial currency accepted = {} PLN", StartTotalAmountRead);

        // StartTotalAmountPaid = TotalAmountPaid = CurrentPaid();
        auto StartTotalAmountPaid = CurrentPaid();
        mik::logger::debug("Initial currency paid out = {} PLN", StartTotalAmountPaid);

        /*-- Determine if there has been an error ---------------------------*/
        auto err_msg = IMHEIConsistencyError(STANDARD_COIN_TIME, STANDARD_NOTE_TIME);
        if (err_msg && *err_msg)
        {
            mik::logger::error("[ERROR] {}", err_msg);
            return false;
        }

        /* The EnableInterface call is used to allow users to enter coins
        or notes into the system. This would be called when a game is
        initialised and ready to accept credit. */
        EnableInterface();

        // CheckOperation() two times must be called, check how does it work

        {
            /*-- Determine total amount paid out --------------------------------*/
            auto amount_paid = (static_cast<double>(CurrentPaid()) / 100.0);
            mik::logger::debug("Amout paid: {}", amount_paid);
        }

        {
            auto platform_type = utils::PlatformType();
            mik::logger::debug("Platform type: [{}] {}", platform_type.first, platform_type.second);
        }

        {
            int serial_number = SerialNumber();
            mik::logger::debug("Serial number: {}", serial_number);
        }

        {
            auto lay_pay = utils::LastPayoutStatus();
            mik::logger::debug("Last pay status: [{}] {}", lay_pay.first, lay_pay.second);
            /*
            TODO: Check PayOut example
            Following a call to PayOut, the programmer should poll this to
            check the progress of the operation .
            */
        }

        {
            char compile_date[16], compile_time[16];
            uint32_t version;
            version = FirmwareVersion(&compile_date[0], &compile_time[0]);
            uint8_t *version_ptr = (uint8_t *)&version;
            mik::logger::debug("Firmware version: {}.{}.{}.{}", version_ptr[3], version_ptr[2], version_ptr[1], version_ptr[0]);
        }

        {
            auto driver_status = utils::USBDriverStatus();
            mik::logger::debug("Driver status: [{}] {}", driver_status.first, driver_status.second);
        }

        {
            bool switch7_state = SwitchOpens(7) == SwitchCloses(7);
            bool switch6_state = SwitchOpens(6) == SwitchCloses(6);
            mik::logger::debug("Switch 6 state: {}", switch6_state == true ? "open" : "closed");
            mik::logger::debug("Switch 7 state: {}", switch7_state == true ? "open" : "closed");
        }

        {
            auto status = utils::DESStatus();
            mik::logger::debug("DESStatus [{}] {}", status.first, status.second);
        }

        return true;
    }

    void system::update_data(std::stop_token stop_token, std::chrono::milliseconds interval)
    {
        EventDetailBlock event_details;
        while (!stop_token.stop_requested())
        {
            if (uint32_t amount_read = CurrentValue())
            {
                /*-- Determine if this value has changed --------------------------*/
                if (amount_read != TotalAmountRead)
                {
                    TotalAmountRead = amount_read;
                    amount_read = TotalAmountRead - StartTotalAmountRead;
                    if (banknote_callback)
                    {
                        pool.detach_task([this,amount_read]
                                         {
                                             banknote_callback(TotalAmountRead, amount_read);
                                         });
                    }
                }
            }

            // /*-- Determine total amount paid out --------------------------------*/
            // if (auto amount_paid = (static_cast<double>(CurrentPaid()) / 100.0))
            // {
            //     ;
            // }

            auto event = NextEvent(&event_details);
            if (event != IMHEI_NULL)
            {
                auto text = decode_event(event);
                if (event >= COIN_DISPENSER_EVENT)
                {
                    if (event_details.DispenserEvent == 0)
                    {
                        mik::logger::debug("Event: {}, Raw Code: {:2X}, Acc: {:d}", text, event_details.RawEvent, event_details.Index);
                    }
                    else
                    {
                        mik::logger::debug("Event: {}, Raw Code: {:2X}, Disp: {:d}", text, event_details.RawEvent, event_details.Index);
                    }
                }
                else
                {
                    mik::logger::debug("Event: {}, ", text);
                }
            }
            mik::logger::trace("sleep_for(2s)");
            std::this_thread::sleep_for(interval);
        }
    }

    int system::dispense_coins(uint32_t amount)
    {
        //TODO: avoid concurrent calls
        auto primary_paid = CurrentPaid();
        auto primary_dispensed_coins = coin_dispenser.getDispensedCoins();
        PayOut(amount);

        while (LastPayStatus() == PAY_ONGOING)
        {
            // if (CurrentPayOut != CurrentPaid())
            // {
            //     CurrentPayOut = CurrentPaid();
            //     mik::logger::debug("      Now paid out: {}", CurrentPayOut);
            // }
            // // mik::logger::debug("sleep_for(20ms)", CurrentPayOut);

            std::this_thread::sleep_for(500ms);
        }

        if (LastPayStatus() != PAY_FINISHED)
        {
            mik::logger::error("Error {} when paying coins", LastPayStatus());
            mik::logger::error("        Total value paid out: {}", CurrentPaid());
            return -1;
        }
        else
        {
            auto dispensed_coins = coin_dispenser.getDispensedCoins() - primary_dispensed_coins;
            auto paid_out = CurrentPaid() - primary_paid;
            mik::logger::debug("coins paid out, Value {} : Coins {}", paid_out, dispensed_coins);
            return dispensed_coins;
        }
    }

    std::string system::version()
    {
        return std::format("{} {}.{}.{}", PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    }

    uint32_t system::level_of_coins()
    {
        return coin_dispenser.getLevelOfCoins();
    }

    uint32_t system::current_credit()
    {
        return CurrentValue();
    }

    system::~system()
    {
        if (worker_thread.joinable())
        {
            mik::logger::trace("worker_thread.join()");
            worker_thread.request_stop();
            worker_thread.join();
        }
        /* The DisableInterface call is used to prevent users from
        entering any more coins or notes. */
        DisableInterface();
        mik::logger::trace("DisableInterface");
    }

}