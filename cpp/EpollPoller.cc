//c++ program
//design by >^.^<
//@date    2019-06-13 05:10:05
#include "EpollPoller.hh"
#include "Handler.hh"

#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>

EpollPoller::EpollPoller()
: activeEventList_(kInitEventListSize)
{
    epfd_ = epoll_create1(EPOLL_CLOEXEC);
}

EpollPoller::~EpollPoller()
{
    close(epfd_);
}

void
EpollPoller::update(Handler* handler)
{
    epoll_event epEvent;
    epEvent.events = handler->events();
    epEvent.data.ptr = handler;
    int op;
    if(handler->isInEpoll)
    {
        op = EPOLL_CTL_MOD;
    }
    else{
        op = EPOLL_CTL_ADD;
    }
    handler->isInEpoll = true;
    int ret = epoll_ctl(epfd_, op, handler->fd(), &epEvent);
    if(ret < 0){
        perror("epoll_ctl ADD");
        printf("epoll_ctl error on fd = %d\n", handler->fd());
    }
    else{
        printf("epoll_ctl on fd = %d success\n", handler->fd());
    }
}

void 
EpollPoller::remove(Handler* handler)
{
    assert(handler->isInEpoll);
    handler->isInEpoll = false;
    int op = EPOLL_CTL_DEL;
    int ret = epoll_ctl(epfd_, op, handler->fd(), NULL);
    if(ret < 0){
        perror("epoll_ctl DEL");
        printf("epoll_ctl error on fd = %d\n", handler->fd());
    }
    handler->isInEpoll = false;
}

std::vector<Handler*>
EpollPoller::fillActiveChannels(int activeNums)
{
    std::vector<Handler*> activeEventPtrList;
    for(int i = 0; i < activeNums; ++i)
    {
        Handler *handlerPtr = static_cast<Handler*>(activeEventList_[i].data.ptr);
        if(activeEventList_[i].events & EPOLLIN)
        {
            handlerPtr->revents_ = EPOLLIN;
        }
        if(activeEventList_[i].events & EPOLLOUT)
        {
            handlerPtr->revents_ = EPOLLOUT;
        }
        activeEventPtrList.push_back(handlerPtr);
    }
    return activeEventPtrList;
}
std::vector<Handler*> 
EpollPoller::poll()
{
    std::vector<Handler*> activeEventPtrList;
    int nreadys = epoll_wait(epfd_, activeEventList_.data(), activeEventList_.size(), -1);
    if(nreadys > 0)
    {
        // 这里需要将就绪的eventList传递到Reactor中
        activeEventPtrList = fillActiveChannels(nreadys);
        if(nreadys == activeEventList_.size())
        {
            activeEventList_.resize(activeEventList_.size() * 2);
        }
    }
    else if(nreadys == 0)
    {
        // nothing happend
    }
    else{
        printf("nreadys = %d, error\n", nreadys);
    }
    return activeEventPtrList;
}
