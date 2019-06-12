//c++ program
//design by >^.^<
//@date    2019-06-11 18:27:05
#pragma once

// 想了想生命周期的管理，主要由于俩点，一是对端断开连接后，一是因为超时被Reactor踢掉

#include <iostream>
#include <boost/noncopyable.hpp>
#include "Callback.hh"

class Reactor;
class Handle : boost::noncopyable
{
public:
    Handle(Reactor *r, int fd);
    ~Handle();
    void handleEvent();
    void setReadCallback();
    void setWriteCallback();
    void setCloseCallback();
    void enableRead();
    void enableWrite();
    void enableAll();
    void disableAll();
    int fd() const;
    int events() const;
    int revents() const;
private:
    int fd_;
    int events_;
    int revents_;
    bool status_; // 是否还在epoll的兴趣列表中
    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;
};
