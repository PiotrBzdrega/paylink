#pragma once
#include <thread>
#include "acceptor.h"
// TODO should composite acceptor, dispenser, paylink, stmf4
namespace paylink
{
class system
{
private:
    bool init();
    std::jthread worker_thread;
    acceptor note_acceptor;
    void get_events(std::stop_token stop_token);
public:
    system(/* args */);
    ~system();
};


}