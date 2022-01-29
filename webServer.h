//
// Created by lx on 1/18/22.
//

#ifndef WEBSERVER_WEBSERVER_H
#define WEBSERVER_WEBSERVER_H

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unordered_map>
#include <thread>
#include <cassert>
#include "utils/utils.h"
#include "threadPool/threadPool.h"
#include "config/config.h"
#include "timerManage/timerManage.h"
#include "timerManage/timer/timer.h"

using namespace std;

class Timer;
struct client_data;
class TimerManage;

class WebServer {
public:
    WebServer();
    ~WebServer();


    void eventListen();
    void eventLoop();
    bool dealClientLink();
    bool dealWithSig();
    void dealWithRead(int sockfd);
    void dealWithWrite(int sockfd);
    void timer(int sockfd, sockaddr_in client_address);  // 定时器初始化
    void dealTimer(Timer *timer, int sockfd);
    void adjustTimer(Timer *timer);


private:
    int m_serv_sock;  // 服务器监听端口
    int m_epfd;  // epoll描述符
    int m_port = DEFAULT_PORT;
    sockaddr_in m_serv_addr;
    HttpConnection *m_users;
    ThreadPool<HttpConnection> *m_pool;
    epoll_event *m_ep_events = new epoll_event[EPOLL_SIZE];
    shared_ptr<Utils> m_utils;
    char *m_root;
    int m_pipefd[2];  // 用于传输信号
    bool m_timeout = false;
    bool m_exit = false;
    
    TimerManage *m_tm;
    vector<client_data> m_users_timer;

    int m_listen_mode = 0;  // 0: LT   1: ET
    int m_trig_mode = 1;
    int m_act_mode = 1;  // 1: reactor   0: proactor
};
#endif //WEBSERVER_WEBSERVER_H
