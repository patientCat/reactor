#pragma once

#include <boost/noncopyable.hpp>
#include <vector>

#include <sys/epoll.h>
#include <memory>


class Handler;
class EpollPoller : boost::noncopyable
{
    typedef std::vector<epoll_event>EventList;
public:
    EpollPoller();
    ~EpollPoller();
    void update(Handler*);
    void remove(Handler*);
    std::vector<Handler*> poll();
    
    static const int kInitEventListSize = 16;
private:
    std::vector<Handler*> fillActiveChannels(int);
private:
    int epfd_;
    EventList activeEventList_;
};
