#include "system.h"
#include "utils.h"
#include "ImheiEvent.h"
#include "logger.h"
#include "version.h"
#include <chrono>
#include <functional>
#include <ranges>

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
    system::system() : nfc_reader(pool), stm32(pool), sensors(pool)
    {
        if (init())
        {
            create map with vendor/product id and let the function fills it
            mik::logger::debug("{}", utils::getSerialPortPath());
            put here strings from function above
            stm32.run_communication("", "");

            /* Start interval functions */
            {
                /* BANKNOTE */
                scheduler.submit_periodic_task(
                    [this](/* this auto& self */)
                    { update_banknote(); },
                    500ms);

                /* EVENTS */
                scheduler.submit_periodic_task(
                    [this](/* this auto& self */)
                    { update_event(); },
                    1s);

                /* BUTTONS */
                scheduler.submit_periodic_task(
                    [this]()
                    { sensors.get_buttons_state(true); },
                    100ms);
            }

            // if (coin_dispenser.setup() == false)
            // {
            //     // TODO: how to handle paylink DisableInterface if exception thrown
            //     throw std::runtime_error("Dispenser setup failed");
            // }
        }
    }

    void system::set_new_banknote_callback(cb::BanknoteCallback func)
    {
        banknote_callback = func;
    }

    int system::set_card_detected_callback(cb::CardDetectionCallback func)
    {
        return nfc_reader.poll(func);
    }

    void system::set_buttons_state_change_callback(cb::ButtonsChangeCallback func)
    {
        sensors.buttons_callback = func;
    }

    void system::set_sensors_state_change_callback(cb::SignalChangeCallback func)
    {
        stm32.set_sensors_state_change_callback(func);
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

        /* Initialize buttons state */
        sensors.get_buttons_state(false);

        /* The EnableInterface call is used to allow users to enter coins
        or notes into the system. This would be called when a game is
        initialised and ready to accept credit. */
        EnableInterface();

        // CheckOperation() two times must be called, check how does it work

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
            auto status = utils::DESStatus();
            mik::logger::debug("DESStatus [{}] {}", status.first, status.second);
        }

        return true;
    }

    void system::update_banknote()
    {
        /* Check if new banknote appears */
        if (auto new_banknote_read = CurrentValue() - StartTotalAmountRead)
        {
            /* Add to sum */
            TotalAmountRead += new_banknote_read;

            /* Callback available ? */
            if (banknote_callback)
            {
                pool.detach_task([this, new_banknote_read]
                                 { banknote_callback(TotalAmountRead, new_banknote_read); });
            }
        }
    }

    void system::update_event()
    {
        EventDetailBlock event_details;
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
    }

    int system::dispense_coins(uint32_t amount)
    {
        /* This call must be exclusive to disallow concurrent calls */
        std::lock_guard<std::mutex> lck(mtx_dispense_coins);

        auto prepay_prom = std::promise<std::pair<int, int>>{};
        auto prepay_fut = prepay_prom.get_future();

        /* Payment Task */
        scheduler.submit_task([this, amount, prom = std::move(prepay_prom)] mutable
                              {
            auto primary_paid = CurrentPaid();
            auto primary_dispensed_coins = coin_dispenser.getDispensedCoins();
            PayOut(amount);
            prom.set_value(std::make_pair(primary_paid,primary_dispensed_coins)); });

        /* Wait for pre-payment result */
        auto [prepay_paid, prepay_dispensed_coins] = prepay_fut.get();

        int postpay_paid{};
        int postpay_dispensed_coins{};

        int pay_st{};
        do
        {
            std::this_thread::sleep_for(500ms);
            auto postpay_prom = std::promise<int>{};
            auto postpay_fut = postpay_prom.get_future();
            /* Post payment Task */
            scheduler.submit_task([this, prom = std::move(postpay_prom), &postpay_paid, &postpay_dispensed_coins]() mutable
                                  {
                                    auto st = LastPayStatus();
                                    postpay_paid = CurrentPaid();
                                    postpay_dispensed_coins = coin_dispenser.getDispensedCoins();
                                    prom.set_value(st); });
            /* Wait for post payment */
            pay_st = postpay_fut.get();
        } while (pay_st == PAY_ONGOING);

        if (pay_st != PAY_FINISHED)
        {
            mik::logger::error("Error {} when paying coins", pay_st);
            mik::logger::error("        Total value paid out: {} coins: {}", postpay_paid, postpay_dispensed_coins);
            return -1;
        }
        else
        {
            auto dispensed_coins = postpay_dispensed_coins - prepay_dispensed_coins;
            auto paid_out = postpay_paid - prepay_paid;
            mik::logger::debug("[Payment SUCCESS] Coins paid out, Value {} : Coins {}", paid_out, dispensed_coins);
            return dispensed_coins;
        }
    }

    uint16_t system::get_buttons_state()
    {
        auto prom = std::promise<int>{};
        auto fut = prom.get_future();
        /* Post payment Task */
        scheduler.submit_task([this, prom = std::move(prom)]() mutable
                              { prom.set_value(sensors.get_buttons_state(true)); });

        return fut.get();
    }

    std::string system::get_sensors_state()
    {
        // TODO: reference to stm32
        return std::string();
    }

    void system::set_led(int number, bool on, uint32_t interval_ms)
    {
        if (led_pending_task_map.contains(number))
        {
            auto task_no = led_pending_task_map[number];
            /* Remove pending task if already exists */
            scheduler.remove_task(task_no);

            /* Remove task number from map */
            led_pending_task_map.erase(number);
        }

        if (interval_ms > 0)
        {
            /*Blink */
            led_pending_task_map[number] = scheduler.submit_periodic_task(
                [number, on]() mutable
                {
                    on ? IndicatorOn(number) : IndicatorOff(number);
                    on = !on;
                },
                std::chrono::milliseconds(interval_ms));
        }
        else
        {
            /* Switch ON LED */
            scheduler.submit_task(
                [number, on]()
                {
                    on ? IndicatorOn(number) : IndicatorOff(number);
                });
        }
    }

    void system::set_motor(bool on, uint32_t ms)
    {

        /* This call must be exclusive to disallow concurrent calls */
        std::lock_guard<std::mutex> lck(mtx_set_motor);

        static constexpr int MOTOR_OUTPUT{0};

        /* Switch ON Motor */
        scheduler.submit_task(
            [this, on]()
            {
                on ? IndicatorOn(MOTOR_OUTPUT) : IndicatorOff(MOTOR_OUTPUT);
            });

        if (ms > 0)
        {
            /* Switch OFF Motor (if there is a delay specified) */
            scheduler.submit_task(
                [this, on]()
                {
                    !on ? IndicatorOn(MOTOR_OUTPUT) : IndicatorOff(MOTOR_OUTPUT);
                },
                std::chrono::seconds(ms / 1000));
        }
    }

    std::string system::version()
    {
        return std::format("{} {}.{}.{}", PROJECT_NAME, VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
    }

    int system::level_of_coins()
    {
        auto prom = std::promise<int>{};
        auto fut = prom.get_future();
        /* Post payment Task */
        scheduler.submit_task([this, prom = std::move(prom)]() mutable
                              { prom.set_value(coin_dispenser.getLevelOfCoins()); });

        return fut.get();
    }

    int system::current_credit()
    {
        auto prom = std::promise<int>{};
        auto fut = prom.get_future();
        /* Post payment Task */
        scheduler.submit_task([prom = std::move(prom)]() mutable
                              { prom.set_value(CurrentValue()); });

        return fut.get();
    }

    system::~system()
    {
        // TODO: here we must be sure that scheduler does not run any task
        scheduler.stop();
        /* The DisableInterface call is used to prevent users from
        entering any more coins or notes. */
        DisableInterface();
        mik::logger::trace("DisableInterface");
    }
    uint16_t system::sensors_t::get_buttons_state(bool notify_via_callback)
    {
        uint16_t new_state{};
        uint16_t open_rise{};
        uint16_t close_rise{};
        for (int i : std::views::iota(0, INPUTS_LEN))
        {
            /* OPEN */
            auto new_open_counter = SwitchOpens(i);
            if (open_counter[i] != new_open_counter)
            {
                open_rise |= (1u << i);
                open_counter[i] = new_open_counter;
            }

            /* CLOSE */
            auto new_close_counter = SwitchCloses(i);
            if (close_counter[i] != new_close_counter)
            {
                close_rise |= (1u << i);
                close_counter[i] = new_close_counter;
            }

            if (new_open_counter == new_close_counter)
            {
                new_state |= (1u << i);
            }
        }

        /* Change recognized */
        uint16_t state_change;
        state_change = new_state ^ state;
        if (state_change)
        {
            state = new_state;
        }

        /* Recognize if state or state edge has change */
        if (state_change | open_rise | close_rise)
        {
            if (notify_via_callback && buttons_callback)
            {
                pool.detach_task([this, state_change, open_rise, close_rise]
                                 { buttons_callback(state_change, open_rise, close_rise); });
            }
        }
        return state;
    }
}