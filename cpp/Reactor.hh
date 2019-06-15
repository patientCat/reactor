#pragma once

#include "EpollPoller.hh"
#include <memory>
class Handler;

class Reactor
{
public:
    Reactor()
    : poller_(new EpollPoller)
    {};
    void registerHandler(Handler* );
    void removeHandler(Handler*);
    void loop();
private:
    std::unique_ptr<EpollPoller> poller_;
};
