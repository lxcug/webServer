//
// Created by lx on 1/18/22.
//

#ifndef WEBSERVER_HTTPCONNECTION_H
#define WEBSERVER_HTTPCONNECTION_H
#include <memory>
#include "../utils/utils.h"
#include "parse/parse.h"

class Parse;

class HttpConnection {
public:
    HttpConnection();
    ~HttpConnection();

    void init(int sockfd, sockaddr_in addr, char *root, std::shared_ptr<Utils> utils);
    void close_connection(bool real_close = true);
    bool read_once();  // 读取浏览器送来的数据
    HTTP_CODE process_read();  // 处理请求报文
    bool process_write(HTTP_CODE ret);  // 完成响应报文
    void process();  // 处理请求并完成回送报文
    bool write();  // 将响应报文送至浏览器

    int m_state;  // 读为0，写为1

    static int m_user_count;
public:
    static int m_epfd;
    int m_timer_flag;
    int m_improv;

private:
    int m_sockfd;

    sockaddr_in m_addr;


    std::shared_ptr<Utils> m_utils;
    Parse *m_parse;

    struct iovec m_iv[2];
    int m_iv_count;
};


#endif //WEBSERVER_HTTPCONNECTION_H
