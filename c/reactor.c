//c++ program
//design by >^.^<
//@date    2019-06-11 19:16:09

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <assert.h>

#define MAX_EVENTS 1024
#define BUFFLEN 4096
#define my_error(x, str) \
do{\
    if(x < 0)\
    {   perror(str); exit(-1);}\
}while(0)

void recvdata(int fd, int events, void *arg);
void senddata(int fd, int events, void *arg);
typedef void (*Callback_t)(int , int , void*);
struct myevent_s{
    int fd;
    int events;
    void *arg;
    Callback_t call_back;
    int status;
    char buf[BUFFLEN];
    int len;
    long last_active;
};

int g_efd;
struct myevent_s g_events[MAX_EVENTS+1];

void eventReset(myevent_s *ev)
{
    bzero(ev->buf, sizeof ev->buf);
    ev->len = 0;
    ev->last_active = time(NULL);
}

void eventSet(myevent_s *ev, int fd, int events, Callback_t call_back, void * arg)
{
    ev->fd = fd;
    ev->call_back = call_back;
    ev->events = events;
    ev->arg = arg;
    ev->last_active = time(NULL);
    return ;
}

void eventDel(int efd, struct myevent_s* ev)
{
    if(ev->status != 1)
        return ;
    epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, NULL);
    ev->status = 0;
}
void eventAdd(int efd, struct myevent_s* ev)
{
    struct epoll_event epv;
    bzero(&epv, sizeof epv);
    int op;
    epv.data.ptr = ev;
    epv.events = ev->events;
    if(ev->status == 1){
        op = EPOLL_CTL_MOD;
    }
    else{
        op = EPOLL_CTL_ADD;
        ev->status = 1;
    }
    if(epoll_ctl(efd, op, ev->fd, &epv) < 0)
    {
        printf("epoll_ctl failed\n");
    }
    else{
        printf("epoll_ctl [fd = %d] events[%0x] success\n", ev->fd, ev->events);
    }
    return ;
}
void senddata(int fd, int events, void *arg)
{
    struct myevent_s *ev = (struct myevent_s*) arg;
    assert(fd == ev->fd);
    printf("fd = %d\n", fd);
    int nbytes = write(ev->fd, ev->buf, ev->len);
    printf("nbytes = %d\n", nbytes);
    eventDel(g_efd, ev);
    if(nbytes > 0)
    {
        eventSet(ev, ev->fd, EPOLLIN, recvdata, ev);
        eventAdd(g_efd, ev);
    }
    else{
        close(ev->fd);
        printf("write error\n");
    }
}
void recvdata(int fd, int events, void *arg)
{
    struct myevent_s *ev = (struct myevent_s*) arg;
    assert(fd == ev->fd);
    int nbytes = read(fd, ev->buf, sizeof ev->buf);
    //eventDel(g_efd, ev);
    if(nbytes > 0)
    {
        ev->len = nbytes;
        ev->buf[nbytes] = 0;
        printf("%s\n", ev->buf);
        eventSet(ev, ev->fd, EPOLLOUT, senddata, ev);
        eventAdd(g_efd, ev);
    }
    else if(nbytes == 0)
    {
        close(ev->fd); // close 会直接导致从关注列表remove
        printf("fd:%d closed \n", ev->fd);
        ev->status = 0;
    }
    else
    {
        printf("fd:%d recv error\n", ev->fd);
        ev->status = 0;
    }
    return ;
}
void acceptConn(int fd, int events, void *arg)
{
    // 找一个空闲的event
    int i = 0;
    for(i = 0; i < MAX_EVENTS; ++i)
    {
        if(g_events[i].status == 0)
        {
            break;
        }
    }
    if(i == MAX_EVENTS)
    {
        printf("max events limited\n");
        return;
    }
    int connfd = accept4(fd, NULL, NULL, SOCK_NONBLOCK);
    my_error(connfd, "accept4 error");
    eventSet(&g_events[i], connfd, EPOLLIN, recvdata, &g_events[i]);
    eventAdd(g_efd, &g_events[i]);
}

void initlistensocket(int epfd, unsigned short port)
{
    int lfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    my_error(lfd, "socket error");

    int reuse = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
    struct sockaddr_in addr;
    bzero(&addr, sizeof addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret = ::bind(lfd, (sockaddr*)&addr, sizeof addr);
    my_error(ret, "bind error");

    ret = listen(lfd, 16);
    my_error(ret, "listen error");

    printf("fd= %d\n", lfd);
    eventSet(&g_events[MAX_EVENTS], lfd, EPOLLIN, acceptConn, &g_events[MAX_EVENTS]);
    eventAdd(epfd, &g_events[MAX_EVENTS]);
}

int main()
{
    unsigned short port = 8888;
    g_efd = epoll_create1(EPOLL_CLOEXEC);
    my_error(g_efd, "epoll_create error");

    initlistensocket(g_efd, port);
    struct epoll_event events[MAX_EVENTS+1];
    printf("server running:port[%d]\n", port);

    int checkpos = 0, i;
    while(1)
    {
        // 超时验证
        /*
        long now = time(NULL);
        for(i = 0; i < 100; i++, checkpos++)
        {
            if(checkpos == MAX_EVENTS)
                checkpos = 0;
            if(g_events[checkpos].status != 1)
                continue;

            long duration = now - g_events[checkpos].last_active;
            if(duration >= 60)
            {
                close(g_events[checkpos].fd);
                printf("[fd=%d] timeout\n", g_events[checkpos].fd);
                eventDel(g_efd, &g_events[checkpos]);
            }
        }
        */

        int nfd = epoll_wait(g_efd, events, MAX_EVENTS + 1, 1000);
        my_error(nfd, "epoll_wait error");
        for(i = 0; i < nfd; ++i)
        {
            struct myevent_s *ev = (struct myevent_s *)events[i].data.ptr;
            if((events[i].events & EPOLLIN) && (ev->events & EPOLLIN))
                ev->call_back(ev->fd, events[i].events, ev->arg);
            if((events[i].events & EPOLLOUT) && (ev->events & EPOLLOUT))
                ev->call_back(ev->fd, events[i].events, ev->arg);
        }
    }
    return 0;
}
