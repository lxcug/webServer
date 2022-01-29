//
// Created by lx on 1/18/22.
//
#include "utils.h"
#include <sys/epoll.h>
#include <unistd.h>


void Utils::setNonBlockFd(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag|O_NONBLOCK);
}

void Utils::addfd(int epfd, int fd, bool oneshot, int isET) {
    epoll_event event;
    event.data.fd = fd;
    if(isET)
        event.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    else
        event.events = EPOLLIN | EPOLLRDHUP;
    if(oneshot)
        event.events |= EPOLLONESHOT;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &event);
    setNonBlockFd(fd);
}

void Utils::removefd(int epfd, int fd) {
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
}

void Utils::modfd(int epfd, int fd, int ev, int trig_mode) {
    epoll_event event;
    event.data.fd = fd;
    if(trig_mode == 1)
        event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    else
        event.events = ev | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &event);
}
