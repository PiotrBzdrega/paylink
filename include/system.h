#pragma once
#include <thread>
#include "acceptor.h"
#include "dispenser.h"
#include "pn532.h"
#include "stm.h"
#include "BS_thread_pool.hpp"
#include "callbacks.h"
// TODO should composite acceptor, dispenser, paylink, stmf4, pn542
namespace paylink
{
    constexpr static auto INPUTS_LEN{16};
    class system
    {
    private:
        std::jthread worker_thread;
        acceptor note_acceptor;
        dispenser coin_dispenser;
        BS::thread_pool<> pool{4};
        nfc::pn532 nfc_reader;
        uc::stm stm32;
        cb::BanknoteCallback banknote_callback{nullptr};
        cb::ButtonsChangeCallback buttons_callback{nullptr};
        int TotalAmountRead{};
        int StartTotalAmountRead{};
        uint16_t sensors_state;
        bool init();
        void update_data(std::stop_token stop_token, std::chrono::milliseconds interval);
        uint16_t get_buttons_state(bool detect_change);
        // uint32_t TotalAmountPaid{};
        // uint32_t StartTotalAmountPaid{};

    public:
        // TODO: think through if we need some configuration file
        system(/* args */);
        /* ASYNC */
        void set_new_banknote_callback(cb::BanknoteCallback func);
        int set_card_detected_callback(cb::CardDetectionCallback func);
        void set_buttons_state_change_callback(cb::ButtonsChangeCallback func);
        void set_sensors_state_change_callback(cb::SignalChangeCallback func);
        void set_error_event_callback(cb::ErrorEventCallback func);
        // set callbacks and pass requests to endpoint classes
        /* SYNC */
        int
        dispense_coins(uint32_t amount);
        uint16_t get_buttons_state();
        std::string get_sensors_state();
        std::string set_led(int number, bool on);
        std::string set_motor(bool on);
        std::string version();
        uint32_t level_of_coins();
        uint32_t current_credit();
        ~system();
    };
}

/*
Event: Note: Now Ok, Raw Code: FE, Acc: 0
Event: Hopper: Now OK, Raw Code:  0, Disp: 0
Event: Note: Reject Note, Raw Code:  2, Acc: 0
Event: Note: Note Returned, Raw Code:  1, Acc: 0
*/
