#pragma once
#include <thread>
#include "acceptor.h"
#include "dispenser.h"
#include "pn532.h"
// TODO should composite acceptor, dispenser, paylink, stmf4
namespace paylink
{
    typedef void (*BanknoteCallback)(double overall_val, double banknote_val);
    typedef void (*ErrorEventCallback)(const char* msg);
    typedef void (*CardDetectionCallback)(const char* uid); //consider struct with more info
    typedef void (*SignalChangeCallback)(bool* view, int v_size, int* signals, int s_size);
    class system
    {
    private:
        bool init();
        void update_data(std::stop_token stop_token, std::chrono::seconds interval);
        std::jthread worker_thread;
        acceptor note_acceptor;
        dispenser coin_dispenser;

        BanknoteCallback banknote_callback{nullptr};
        double TotalAmountRead{};
        double StartTotalAmountRead{};

    public:
        system(/* args */);
        void set_banknote_callback(BanknoteCallback func);
        ~system();
    };
}

/*
Event: Note: Now Ok, Raw Code: FE, Acc: 0
Event: Hopper: Now OK, Raw Code:  0, Disp: 0
Event: Note: Reject Note, Raw Code:  2, Acc: 0
Event: Note: Note Returned, Raw Code:  1, Acc: 0
*/
