//
// Created by lx on 1/23/22.
//

#ifndef WEBSERVER_TIMER_H
#define WEBSERVER_TIMER_H
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cassert>
#include <sys/epoll.h>
#include <unistd.h>
#include "../../webServer.h"

struct client_data;

class Timer {
public:
    client_data *m_user_data;
    void (*cb_func)(client_data *);
    time_t expire;
};

struct client_data {
    sockaddr_in address;
    int sockfd;
    Timer *timer;
};


#endif //WEBSERVER_TIMER_H
