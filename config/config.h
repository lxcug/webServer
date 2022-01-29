//
// Created by lx on 1/18/22.
//

#ifndef WEBSERVER_CONFIG_H
#define WEBSERVER_CONFIG_H

const int MAX_FD = 65536;
const int DEFAULT_PORT = 8000;
const int MAX_CONNECTIONS = 5;  // 最大连接的客户端数
const int EPOLL_SIZE = 100;
const int MAX_RQST_NUM = 2048;
const int MAX_THREAD_NUM = 10;
const int READ_BUFFER_SIZE = 2048;
const int WRITE_BUFFER_SIZE = 1024;
const int FILENAME_LEN = 200;
const int TIME_SLOT = 5;

enum METHOD
{
    GET = 0,
    POST,
    HEAD,
    PUT,
    DELETE,
    TRACE,
    OPTIONS,
    CONNECT,
    PATH
};
enum HTTP_CODE
{
    NO_REQUEST,
    GET_REQUEST,
    BAD_REQUEST,
    NO_RESOURCE,
    FORBIDDEN_REQUEST,
    FILE_REQUEST,
    INTERNAL_ERROR,
    CLOSED_CONNECTION
};
enum LINE_STATUS
{
    LINE_OK = 0,
    LINE_BAD,
    LINE_OPEN
};

enum CHECK_STATE
{
    CHECK_STATE_REQUESTLINE = 0,
    CHECK_STATE_HEADER,
    CHECK_STATE_CONTENT
};

#endif //WEBSERVER_CONFIG_H
