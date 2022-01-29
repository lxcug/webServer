//
// Created by lx on 1/18/22.
//

#include "webServer.h"

class Utils;

WebServer::WebServer() {
    m_users = new HttpConnection[MAX_FD];
    m_pool = new ThreadPool<HttpConnection>;
    ThreadPool<HttpConnection>::m_act_mode = m_act_mode;
    // root文件夹路径
    char server_path[200];
    getcwd(server_path, 200);
    char root[6] = "/root";
    m_root = (char *)malloc(strlen(server_path) + strlen(root) + 1);
    strcpy(m_root, server_path);
    strcat(m_root, root);
    m_utils = make_shared<Utils> ();
    m_tm = new TimerManage(m_utils);
    TimerManage::m_pipefd = this->m_pipefd;
    m_users_timer.resize(MAX_FD);
}

WebServer::~WebServer() {
    close(m_epfd);
    close(m_serv_sock);
    delete[] m_ep_events;
    delete[] m_pool;
    close(m_pipefd[0]);
    close(m_pipefd[1]);
    delete m_users;
}

void WebServer::eventListen() {
    m_serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(m_serv_sock >= 0);
    int flag = 1;
    setsockopt(m_serv_sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

    struct linger tmp = {1, 1};
    setsockopt(m_serv_sock, SOL_SOCKET, SO_LINGER, &tmp, sizeof(tmp));

    m_serv_addr.sin_family = PF_INET;
    m_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    m_serv_addr.sin_port = htons(m_port);
    int ret = bind(m_serv_sock, (sockaddr *)&m_serv_addr, sizeof(m_serv_addr));
    assert(ret != -1);
    ret = listen(m_serv_sock, MAX_CONNECTIONS);
    assert(ret != -1);

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipefd);
    assert(ret != -1);
    this->m_epfd = epoll_create((EPOLL_SIZE));
    assert(m_epfd != -1);

    m_utils->setNonBlockFd(m_pipefd[1]);
    m_utils->addfd(m_epfd, m_pipefd[0], 0, 0);

    m_tm->addsig(SIGPIPE, SIG_IGN);
    m_tm->addsig(SIGALRM, m_tm->sig_handler, false);
    m_tm->addsig(SIGTERM, m_tm->sig_handler, false);

    HttpConnection::m_epfd = this->m_epfd;
    m_utils->addfd(m_epfd, m_serv_sock, 0, m_listen_mode);
    TimerManage::m_pipefd = this->m_pipefd;
    alarm(TIME_SLOT);
}

void WebServer::eventLoop() {
    while(!m_exit) {
        int num = epoll_wait(m_epfd, m_ep_events, EPOLL_SIZE, -1);
        for(int i = 0; i < num; i++) {
            int sockfd = m_ep_events[i].data.fd;
            if(sockfd == m_serv_sock) {  // 有新的连接
                bool flag = dealClientLink();
            }
            else if (m_ep_events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {  // 服务器端关闭连接，移除对应的定时器
		cout << "port remove sockfd " << sockfd << " ";
                Timer *timer = m_users_timer[sockfd].timer;
                dealTimer(timer, sockfd);
            }
            else if ((sockfd == m_pipefd[0]) && (m_ep_events[i].events & EPOLLIN)) {  // 处理信号
                bool flag = dealWithSig();
                if(!flag) {
                    //  error hander
                }
            }
            else if(m_ep_events[i].events & EPOLLIN) {  // 读数据
                dealWithRead(sockfd);
            }
            else if(m_ep_events[i].events & EPOLLOUT){  // 写数据
		cout << "write " << sockfd << endl;
                dealWithWrite(sockfd);
            }
        }
        if(m_timeout) {  // 调用tick函数定时删除定时器
	        cout << "time out" << endl;
            m_tm->timer_handler();
            m_timeout = false;
        }
    }
}

void WebServer::dealWithRead(int sockfd) {
    // reactor
     Timer *timer = m_users_timer[sockfd].timer;
     if(m_act_mode == 1) {
         if(timer)
             adjustTimer(timer);  // 延长活跃时间
         m_pool->append(m_users + sockfd, 0);
         while (true) {
             if(m_users[sockfd].m_improv == 1) {
                 if (m_users[sockfd].m_timer_flag == 1) {
                     dealTimer(timer, sockfd);
                     m_users[sockfd].m_timer_flag = 0;
                 }
                 m_users[sockfd].m_improv = 0;
                 break;
             }
         }
     }
    else {  // proactor
         if(m_users[sockfd].read_once()) {
             m_pool->append_p(m_users + sockfd);
             if(timer)
                 adjustTimer(timer);
         }
         else
             dealTimer(timer, sockfd);
    }
}

void WebServer::dealWithWrite(int sockfd) {
    // reactor
    Timer *timer = m_users_timer[sockfd].timer;
    if(m_act_mode == 1) {
        if(timer)
            adjustTimer(timer);
        m_pool->append(m_users + sockfd, 1);
        while (true) {
            if(m_users[sockfd].m_improv == 1) {
                if (m_users[sockfd].m_timer_flag == 1) {
                    dealTimer(timer, sockfd);
                    m_users[sockfd].m_timer_flag = 0;
                }
                m_users[sockfd].m_improv = 0;
                break;
            }
        }
    }
    else {  // proactor
        if(m_users[sockfd].write()) {
            if(timer)
                adjustTimer(timer);
        }
        else
            dealTimer(timer, sockfd);
    }

}

bool WebServer::dealClientLink() {
    sockaddr_in clnt_addr;
    socklen_t clnt_sz = sizeof(clnt_addr);
    int clnt_sock = accept(m_serv_sock, (struct sockaddr *)&clnt_addr, &clnt_sz);
    timer(clnt_sock, clnt_addr);  // 添加定时器
    cout << "clnt sock: " << clnt_sock << endl;
    if (clnt_sock < 0) {
        return false;
    }
    if(HttpConnection::m_user_count >= MAX_FD) {
        return false;
    }
    m_utils->setNonBlockFd(clnt_sock);
    m_users[clnt_sock].init(clnt_sock, clnt_addr, m_root, m_utils);  // 给httpConnection对象初始化监听端口和客户端地址
    m_utils->addfd(m_epfd, clnt_sock, 1, 1);
    return true;
}

bool WebServer::dealWithSig() {
    int ret = 0;
    char signals[1024];
    ret = recv(m_pipefd[0], signals, sizeof(signals), 0);
    if (ret == -1 || ret == 0)
        return false;
    else {
        for (int i = 0; i < ret; ++i) {
            switch (signals[i]) {
                case SIGALRM: {
                    cout << "SIGALRM ";
                    m_timeout = true;
                    break;
                }
                case SIGTERM: {
                    cout << "SIGTERM ";
                    m_exit = true;
                    break;
                }
            }
        }
    }
    return true;
}

void cb_func(client_data *user_data) {  // 关闭客户端连接
    epoll_ctl(HttpConnection::m_epfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    close(user_data->sockfd);
    HttpConnection::m_user_count--;
    cout << "cb func remove sockfd " << user_data->sockfd << "  usr count: " << HttpConnection::m_user_count << endl;
}

void WebServer::timer(int sockfd, sockaddr_in client_address) {
    m_users_timer[sockfd].address = client_address;
    m_users_timer[sockfd].sockfd = sockfd;

    auto *timer = new Timer;

    timer->m_user_data = &m_users_timer[sockfd];
    timer->cb_func = cb_func;
    time_t cur = time(nullptr);
    timer->expire = cur + 3 * TIME_SLOT;
    // 定时器与http连接绑定
    m_users_timer[sockfd].timer = timer;
    // 定时器添加到最小堆
    m_tm->add_timer(timer);
}

void WebServer::dealTimer(Timer *timer, int sockfd) {
    timer->cb_func(&m_users_timer[sockfd]);  // 关闭客户端连接
    if(timer)
        m_tm->del_timer(timer);  // 删除定时器
}

void WebServer::adjustTimer(Timer *timer) {
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIME_SLOT;
    m_tm->adjust_timer();
}





















