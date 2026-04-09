#pragma once
#include <thread>
#include "acceptor.h"
#include "dispenser.h"
#include "pn532.h"
#include "stm.h"
#include "BS_thread_pool.hpp"
#include "callbacks.h"
#include "scheduler.h"
// TODO should composite acceptor, dispenser, paylink, stmf4, pn542
namespace paylink
{
    constexpr static auto INPUTS_LEN{16};
    class system
    {
    private:
        acceptor note_acceptor;
        dispenser coin_dispenser;
        BS::thread_pool<> pool{4};
        nfc::pn532 nfc_reader;
        uc::stm stm32;
        cb::BanknoteCallback banknote_callback{nullptr};
        com::scheduler scheduler;
        std::unordered_map<int,std::size_t> led_pending_task_map;
        int TotalAmountRead{};
        int StartTotalAmountRead{};
        struct sensors_t
        {
            explicit sensors_t(BS::thread_pool<> &pool_) : pool(pool_) {};
            BS::thread_pool<> &pool;
            uint16_t state;
            std::array<int, INPUTS_LEN> open_counter;
            std::array<int, INPUTS_LEN> close_counter;
            cb::ButtonsChangeCallback buttons_callback{nullptr};
            uint16_t get_buttons_state(bool notify_via_callback);
        };
        sensors_t sensors;
        std::mutex mtx_dispense_coins;
        std::mutex mtx_set_motor;
        bool init();
        void update_banknote();
        void update_event();
        void read_configuration(std::string_view config_path);

        // uint32_t TotalAmountPaid{};
        // uint32_t StartTotalAmountPaid{};
    public:
    
        // watch abi talk
        // fill calbacks for all calls
        // test nfc reader phisically
        // TODO: think through if we need some configuration file for input mapings ??
        system() = delete;
        system(std::string_view config_path);
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
        void set_led(int number, bool on, uint32_t interval_ms);
        void set_motor(bool on, uint32_t ms = 0);
        std::string version();
        int level_of_coins();
        int current_credit();
        ~system();
    };
}

/*
Event: Note: Now Ok, Raw Code: FE, Acc: 0
Event: Hopper: Now OK, Raw Code:  0, Disp: 0
Event: Note: Reject Note, Raw Code:  2, Acc: 0
Event: Note: Note Returned, Raw Code:  1, Acc: 0
*/
