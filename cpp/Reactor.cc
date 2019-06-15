//c++ program
//design by >^.^<
//@date    2019-06-13 05:05:56
#include "Reactor.hh"
#include "EpollPoller.hh"
#include "Handler.hh"

void 
Reactor::registerHandler(Handler* handler)
{
    poller_->update(handler);
}

void
Reactor::removeHandler(Handler* handler) {
    poller_->remove(handler);
}

void 
Reactor::loop()
{
    while(1)
    {
        auto activeEventPtrList = poller_->poll();
        if(!activeEventPtrList.empty())
        {
            for(int i = 0; i < activeEventPtrList.size(); ++i)
            {
                activeEventPtrList[i]->handleEvent();
            }
        }
        else{
        }
    }
}
