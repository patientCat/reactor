//c++ program
//design by >^.^<
//@date    2019-06-13 04:59:46
#include "Reactor.hh"
#include "Handler.hh"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <functional>

#define my_error(x, str) \
do{\
    if(x < 0)\
    {   perror(str); exit(-1);}\
}while(0)

using namespace std::placeholders;

class EchoServer
{
public:
    EchoServer(int port)
    : port_(port)
    , base_(new Reactor)
    {
    }

    void start()
    {
        startup();
        base_->loop();
    }
    void startup()
    {
        int lfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        my_error(lfd, "socket error");
        auto lHandler = std::make_shared<Handler>(lfd);
        handleList_[lfd] = lHandler;

        int reuse = 1;
        setsockopt(lHandler->fd(), SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
        sockaddr_in addr;
        bzero(&addr, sizeof addr);
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        int ret = ::bind(lHandler->fd(), (sockaddr*)&addr, sizeof addr);
        my_error(ret, "bind error");

        ret = listen(lHandler->fd(), 16);
        my_error(ret, "listen error");

        lHandler->setReadCallback(std::bind(&EchoServer::accpetConn, this, _1));
        lHandler->enableRead();
        base_->registerHandler(lHandler.get());
    };
private:
    void accpetConn(int fd)
    {
        printf("in accept\n");
        int cfd = accept4(fd, NULL, NULL, SOCK_NONBLOCK);
        my_error(cfd, "accept4");
        auto connHandler = std::make_shared<Handler>(cfd);
        handleList_[cfd] = connHandler;
        connHandler->setReadCallback(std::bind(&EchoServer::readdata, this, _1));
        connHandler->enableRead();
        base_->registerHandler(connHandler.get());
    }

    void readdata(int fd)
    {
        printf("in read\n");
        auto pHandler = handleList_[fd];
        int nbytes = read(fd, pHandler->buff(), MAX_BUFFLEN);
        if(nbytes > 0)
        {
            pHandler->buff()[nbytes] = 0;
            printf("%s\n", pHandler->buff());
            pHandler->setLen(nbytes);
            pHandler->setWriteCallback(std::bind(&EchoServer::senddata, this, _1));
            pHandler->enableWrite();
            base_->registerHandler(pHandler.get());
        }
        else if(nbytes == 0){
            printf("close fd\n");
            ::close(fd);
            base_->removeHandler(pHandler.get());
            handleList_.erase(fd);
        }
        else{
            ::close(fd);
            printf("read error\n");
            base_->removeHandler(pHandler.get());
            handleList_.erase(fd);
        }
    }

    void senddata(int fd)
    {
        auto pHandler = handleList_[fd];
        int nbytes = write(fd, pHandler->buff(), pHandler->length());
        if(nbytes > 0)
        {
            pHandler->setReadCallback(std::bind(&EchoServer::readdata, this, _1));
            pHandler->enableRead();
            base_->registerHandler(pHandler.get());
        }
        else {
            printf("write error\n");
            ::close(fd);
            base_->removeHandler(pHandler.get());
            handleList_.erase(fd);
        }
    }
private:
    int port_;
    std::string ip_;
    std::unique_ptr<Reactor> base_;
    std::map<int, std::shared_ptr<Handler>> handleList_;
};
int main()
{
    EchoServer server(8888);
    server.start();
}
