#pragma once
#include <thread>
#include "acceptor.h"
#include "dispenser.h"
// TODO should composite acceptor, dispenser, paylink, stmf4
namespace paylink
{
    class system
    {
    private:
        bool init();
        std::jthread worker_thread;
        acceptor note_acceptor;
        dispenser coin_dispenser;
        void get_events(std::stop_token stop_token);

    public:
        system(/* args */);
        ~system();
    };

}

/* 
Event: Note: Now Ok, Raw Code: FE, Acc: 0
Event: Hopper: Now OK, Raw Code:  0, Disp: 0
Event: Note: Reject Note, Raw Code:  2, Acc: 0
Event: Note: Note Returned, Raw Code:  1, Acc: 0
*/
