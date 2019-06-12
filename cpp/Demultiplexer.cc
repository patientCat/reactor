//c++ program
//design by >^.^<
//@date    2019-06-11 18:48:41
#include "Demultiplexer.hh"
#include "Handle.hh"
#include <sys/epoll.h>
#include <unistd.h>

Demultiplexer::Demultiplexer()
{
    epfd_ = epoll_create1(EPOLL_CLOEXEC);
}
Demultiplexer::~Demultiplexer()
{
    close(epfd_);
}

void 
Demultiplexer::addEvent(Handle* handle)
{
    epoll_event ep_event;
    ep_event.events = handle->events();
    ep_event.data.ptr;
    epoll_ctl(epfd_, EPOLL_CTL_ADD, handle->fd(), );
}

void Demultiplexer::removeEvent()
{
}

void Demultiplexer::modifyEvent()
{
}
