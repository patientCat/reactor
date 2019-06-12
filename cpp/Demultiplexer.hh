#pragma once

// 封装了epoll

#include <boost/noncopyable.hpp>

class Handle;

class Demultiplexer : boost::noncopyable
{
public:
    Demultiplexer();
    ~Demultiplexer();

    void addEvent(Handle*);
    void removeEvent();
    void modifyEvent();
private:
    int epfd_;
};
