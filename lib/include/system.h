#pragma once
#include <thread>
#include "acceptor.h"
#include "dispenser.h"
#include "pn532.h"
#include "stm.h"
#include "logger.h"
#include "BS_thread_pool.hpp"
#include "paylink/callbacks.h"
#include "scheduler.h"

// TODO: add thirdparty include directory to keep current version of only-header libraries
namespace paylink
{
    constexpr static auto INPUTS_LEN{16};
    class system
    {
    private:
        std::ofstream log_file;
        acceptor note_acceptor;
        dispenser coin_dispenser;
        BS::thread_pool<> pool{4};
        nfc::pn532 nfc_reader;
        uc::stm stm32;
        BanknoteCallback banknote_callback{nullptr};
        com::scheduler scheduler;
        std::unordered_map<int, std::size_t> led_pending_task_map;
        int TotalAmountRead{};
        int StartTotalAmountRead{};
        struct sensors_t
        {
            explicit sensors_t(BS::thread_pool<> &pool_) : pool(pool_) {};
            BS::thread_pool<> &pool;
            uint16_t state;
            std::array<int, INPUTS_LEN> open_counter;
            std::array<int, INPUTS_LEN> close_counter;
            ButtonsChangeCallback buttons_callback{nullptr};
            uint16_t get_buttons_state(bool notify_via_callback);
        };
        sensors_t sensors;
        std::mutex mtx_dispense_coins;
        std::mutex mtx_set_motor;
        struct ConfigT
        {
            struct LoggerT
            {
                mik::LogLevel level{mik::LogLevel::INFO};
                bool standard_out{false};
                std::optional<std::string> file_path{};
            } logger;
            struct module_t
            {
                enum class ModType
                {
                    ENABLED,
                    DISABLED,
                    STUB
                };
                ModType paylink{};
                ModType acceptor{};
                ModType dispenser{};
                ModType pn532{};
                ModType stm{};
            } module;
        } config;
        bool init();
        void update_banknote();
        void update_event();
        void read_configuration(std::string_view config_path);

        // uint32_t TotalAmountPaid{};
        // uint32_t StartTotalAmountPaid{};
    public:
        // fill calbacks for all calls
        // test nfc reader phisically
        system() = delete;
        system(std::string_view config_path);
        /* ASYNC */
        void set_new_banknote_callback(BanknoteCallback func);
        int set_card_detected_callback(CardDetectionCallback func);
        void set_buttons_state_change_callback(ButtonsChangeCallback func);
        void set_sensors_state_change_callback(SignalChangeCallback func);
        void set_error_event_callback(ErrorEventCallback func);
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
