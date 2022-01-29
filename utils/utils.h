//
// Created by lx on 1/18/22.
//

#ifndef WEBSERVER_UTILS_H
#define WEBSERVER_UTILS_H

#include <fcntl.h>
#include <cassert>

using namespace std;

class Utils {
public:
    void setNonBlockFd(int fd); // 将文件描述符设为非阻塞模式
    void addfd(int epfd, int fd, bool oneshot, int isET);  // 有新连接时增加epoll监听fd
    void removefd(int epfd, int fd);
    void modfd(int epfd, int fd, int ev, int trig_mode);
    void addsig(int sig, void(handler)(int), bool restart);
    static void sig_handler(int sig);  // 信号处理函数，用m_pipefd将信号传给主函数

public:
    static int m_epollfd;
};

#endif //WEBSERVER_UTILS_H
