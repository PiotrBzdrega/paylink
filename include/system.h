#pragma once
#include <thread>
#include "acceptor.h"
#include "dispenser.h"
#include "pn532.h"
#include "stm.h"
#include "BS_thread_pool.hpp"
// TODO should composite acceptor, dispenser, paylink, stmf4, pn542
namespace paylink
{

    extern "C"
    {
        typedef void (*BanknoteCallback)(double overall_val, double banknote_val);
        typedef void (*ErrorEventCallback)(const char *msg);
        typedef void (*SignalChangeCallback)(bool *view, int v_size, int *signals, int s_size);
    }

    class system
    {
    private:
        std::jthread worker_thread;
        acceptor note_acceptor;
        dispenser coin_dispenser;
        BS::thread_pool<> pool{4};
        nfc::pn532 nfc_reader;
        uc::stm stm32;
        BanknoteCallback banknote_callback{nullptr};
        uint32_t TotalAmountRead{};
        uint32_t StartTotalAmountRead{};
        bool init();
        void update_data(std::stop_token stop_token, std::chrono::milliseconds interval);
        // uint32_t TotalAmountPaid{};
        // uint32_t StartTotalAmountPaid{};


    public:
    //TODO: think through if we need some configuration file
        system(/* args */);
        /* ASYNC */
        void set_banknote_callback(BanknoteCallback func);

        /* SYNC */
        int dispense_coins(uint32_t amount);
        std::string version();
        uint32_t level_of_coins();
        uint32_t current_credit();
        void nfc_poll_card(nfc::CardDetectionCallback cb, int timeout_sec);
        ~system();
    };
}

/*
Event: Note: Now Ok, Raw Code: FE, Acc: 0
Event: Hopper: Now OK, Raw Code:  0, Disp: 0
Event: Note: Reject Note, Raw Code:  2, Acc: 0
Event: Note: Note Returned, Raw Code:  1, Acc: 0
*/
