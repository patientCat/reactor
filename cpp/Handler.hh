//c++ program
//design by >^.^<
//@date    2019-06-11 18:27:05
#pragma once

// 想了想生命周期的管理，主要由于俩点，一是对端断开连接后，一是因为超时被Reactor踢掉

#include <iostream>
#include <boost/noncopyable.hpp>
#include "Callback.hh"
//#include "Buffer.hh"  // 暂时先不不去做Buffer类，一步一步来

#define MAX_BUFFLEN 1024
class Handler : boost::noncopyable
{
    friend class EpollPoller;
public:
    Handler(int );
    ~Handler();
    void handleEvent();
    void setReadCallback(EventCallback);
    void setWriteCallback(EventCallback);
    void setCloseCallback(EventCallback);
    void enableRead();
    void enableWrite();
    void enableAll();
    void disableAll();
    int fd() const{
        return fd_;
    }
    int events() const
    {
        return events_;
    }
    int revents() const
    {
        return revents_;
    }
    char *buff()
    {
        return buff_;
    }
    void setLen(int len)
    { 
        len_ = len;
    }
    int length()
    {
        return len_;
    }
private:
    int fd_;
    int events_;
    int revents_;
    bool isInEpoll; // 是否还在epoll的兴趣列表中
    char buff_[MAX_BUFFLEN];
    int len_;
    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};
