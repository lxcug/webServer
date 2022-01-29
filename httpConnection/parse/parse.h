//
// Created by lx on 1/19/22.
//

#ifndef WEBSERVER_PARSE_H
#define WEBSERVER_PARSE_H
#include <fstream>
#include <unistd.h>
#include <csignal>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cassert>
#include <sys/stat.h>
#include <cstring>
#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <sys/mman.h>
#include <cstdarg>
#include <cerrno>
#include <sys/wait.h>
#include <sys/uio.h>
#include "../../config/config.h"
#include "../httpConnection.h"

class Parse {
public:
    friend class HttpConnection;
    Parse(char *root);
    ~Parse();

    void init();

    LINE_STATUS parse_line();
    HTTP_CODE parse_request_line(char *text);  // 解析http请求行，获得请求方法，目标url及http版本号
    HTTP_CODE parse_headers(char *text);
    HTTP_CODE parse_content(char *text);
    HTTP_CODE do_request();

    inline char *get_line() {
        return m_read_buf + m_start_line;
    };


    void unmap();
    bool add_response(const char *format, ...);
    bool add_content(const char *content);
    bool add_status_line(int status, const char *title);
    bool add_headers(int content_len);
    bool add_content_type();
    bool add_content_length(int content_len);
    bool add_linger();
    bool add_blank_line();

private:
    char m_read_buf[READ_BUFFER_SIZE], m_write_buf[WRITE_BUFFER_SIZE];
    int m_start_line;
    int m_checked_idx;
    int m_write_idx, m_read_idx;
    char *m_url;
    METHOD m_method;
    CHECK_STATE m_check_state;

    char m_real_file[FILENAME_LEN];
    char *m_version;
    char *m_host;
    int m_content_length;
    bool m_linger;
    char *m_string, *doc_root;
    int m_bytes_to_send, m_bytes_have_send;

    char *m_file_address;        //读取服务器上的文件地址
    struct stat m_file_stat;
};


#endif //WEBSERVER_PARSE_H
