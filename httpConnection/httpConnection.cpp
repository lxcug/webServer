//
// Created by lx on 1/18/22.
//

#include <iostream>
#include "httpConnection.h"

const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file form this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";

int HttpConnection::m_epfd = -1;
int HttpConnection::m_user_count = 0;

HttpConnection::HttpConnection() {}

HttpConnection::~HttpConnection() {
    delete m_parse;
};

void HttpConnection::init(int sockfd, sockaddr_in addr, char *root, std::shared_ptr<Utils> utils) {
    m_sockfd = sockfd;
    m_addr = addr;
    m_parse = new Parse(root);
    HttpConnection::m_utils = utils;
    m_parse->init();
    m_timer_flag = 0;
    m_improv = 0;
    m_user_count++;
    cout << "usr count: "<< m_user_count << endl;
}


void HttpConnection::close_connection(bool real_close) {
    if (real_close && m_sockfd != -1) {
        m_utils->removefd(m_epfd, m_sockfd);
	    cout << "close sockfd " << m_sockfd;
        m_sockfd = -1;
        m_user_count--;
	    cout << " usr count" << m_user_count << endl;
    }
}

// 读入m_read_buf
bool HttpConnection::read_once() {
    if(m_parse->m_read_idx >= READ_BUFFER_SIZE)
        return false;
    while(true) {
        int bytes_read = recv(m_sockfd, m_parse->m_read_buf + m_parse->m_read_idx, READ_BUFFER_SIZE - m_parse->m_read_idx, 0);
        if(bytes_read == -1) {  // recv返回-1并且errno = EAGN时无数据可读
            if(errno == EAGAIN)  // || errno == EWOULDBLOCK
                break;
        }
        else if (bytes_read == 0)
            return false;
        m_parse->m_read_idx += bytes_read;
    }
    return true;
}

bool HttpConnection::write() {
    int temp = 0;
    if (m_parse->m_bytes_to_send == 0)
    {
        m_utils->modfd(m_epfd, m_sockfd, EPOLLIN, 1);
        m_parse->init();
        m_timer_flag = 0;
        m_improv = 0;
        return true;
    }
    while(1)
    {
        temp = writev(m_sockfd, m_iv, m_iv_count);
        if(temp < 0)
        {
            if(errno == EAGAIN)
            {
                m_utils->modfd(m_epfd, m_sockfd, EPOLLOUT, 1);
                return true;
            }
            m_parse->unmap();
            return false;
        }

        m_parse->m_bytes_have_send += temp;
        m_parse->m_bytes_to_send -= temp;
        if(m_parse->m_bytes_have_send >= m_iv[0].iov_len)
        {
            m_iv[0].iov_len = 0;
            m_iv[1].iov_base = m_parse->m_file_address + (m_parse->m_bytes_have_send - m_parse->m_write_idx);
            m_iv[1].iov_len = m_parse->m_bytes_to_send;
        }
        else
        {
            m_iv[0].iov_base = m_parse->m_write_buf + m_parse->m_bytes_have_send;
            m_iv[0].iov_len = m_iv[0].iov_len - m_parse->m_bytes_have_send;
        }
        if(m_parse->m_bytes_to_send <= 0)
        {
            m_parse->unmap();
            m_utils->modfd(m_epfd, m_sockfd, EPOLLIN, 1);

            if(m_parse->m_linger)
            {
                m_parse->init();
                m_timer_flag = 0;
                m_improv = 0;
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    return false;
}

HTTP_CODE HttpConnection::process_read() {
    LINE_STATUS line_status = LINE_OK;
    HTTP_CODE ret = NO_REQUEST;
    char *text;

    while ((m_parse->m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) || ((line_status = m_parse->parse_line()) == LINE_OK)) {
        text = m_parse->get_line();
        m_parse->m_start_line = m_parse->m_checked_idx;
        switch (m_parse->m_check_state)
        {
            case CHECK_STATE_REQUESTLINE:
            {
                ret = m_parse->parse_request_line(text);
                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;
                break;
            }
            case CHECK_STATE_HEADER:
            {
                ret = m_parse->parse_headers(text);
                if (ret == BAD_REQUEST)
                    return BAD_REQUEST;
                else if (ret == GET_REQUEST)
                {
                    return m_parse->do_request();
                }
                break;
            }
            case CHECK_STATE_CONTENT:
            {
                ret = m_parse->parse_content(text);
                if (ret == GET_REQUEST)
                    return m_parse->do_request();
                line_status = LINE_OPEN;
                break;
            }
            default:
                return INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}

bool HttpConnection::process_write(HTTP_CODE ret) {
    switch (ret) {
        case INTERNAL_ERROR: {
            m_parse->add_status_line(500, error_500_title);
            m_parse->add_headers(strlen(error_500_form));
            if(!m_parse->add_content(error_500_form))
                return false;
            break;
        }
        case BAD_REQUEST: {
            m_parse->add_status_line(404, error_404_title);
            m_parse->add_headers(strlen(error_404_form));
            if (!m_parse->add_content(error_404_form))
                return false;
            break;
        }
        case FORBIDDEN_REQUEST: {
            m_parse->add_status_line(403, error_403_title);
            m_parse->add_headers(strlen(error_403_form));
            if(!m_parse->add_content(error_403_form))
                return false;
            break;
        }
        case FILE_REQUEST: {
            m_parse->add_status_line(200, ok_200_title);
            if(m_parse->m_file_stat.st_size != 0) {
                m_parse->add_headers(m_parse->m_file_stat.st_size);
                m_iv[0].iov_base = m_parse->m_write_buf;
                m_iv[0].iov_len = m_parse->m_write_idx;
                m_iv[1].iov_base = m_parse->m_file_address;
                m_iv[1].iov_len = m_parse->m_file_stat.st_size;
                m_iv_count = 2;
                m_parse->m_bytes_to_send = m_parse->m_write_idx + m_parse->m_file_stat.st_size;
                return true;
            }
            else {
                const char *ok_string = "<html><body></body></html>";
                m_parse->add_headers(strlen(ok_string));
                if(!m_parse->add_content(ok_string))
                    return false;
            }
        }
        default:
            return false;
    }
    m_iv[0].iov_base = m_parse->m_write_buf;
    m_iv[0].iov_len = m_parse->m_write_idx;
    m_iv_count = 1;
    m_parse->m_bytes_to_send = m_parse->m_write_idx;
    return true;
}

void HttpConnection::process() {
    HTTP_CODE read_ret = process_read();
    if(read_ret == NO_REQUEST) {
        //注册并监听读事件
        m_utils->modfd(m_epfd, m_sockfd, EPOLLIN, 1);
        return;
    }
    // 调用process_write完成报文响应
    bool write_ret = process_write(read_ret);
    //if(!write_ret)
	// close_connection()但是必须删除定时器
    // 注册并监听写事件
    m_utils->modfd(m_epfd, m_sockfd, EPOLLOUT, 1);
}
