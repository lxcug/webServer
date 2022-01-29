//
// Created by lx on 1/19/22.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H

#include <queue>
#include <thread>
#include <mutex>
#include <vector>
#include <iostream>
#include <condition_variable>
#include "../httpConnection/httpConnection.h"
#include "../config/config.h"


template<class T>
class ThreadPool {
public:
    ThreadPool();
    ~ThreadPool();

    bool append(T *request, int state);
    bool append_p(T *request);

    void run();
    static void* worker(ThreadPool *p);


    static int m_act_mode;
private:
    int m_thread_num;  // 线程池中的线程数
    int m_max_rqst_num;  // 请求队列中的最大请求数
    int m_stop;
    std::vector<std::thread> m_threads;
    std::queue<T*> m_rqst_queue;  // 请求队列
    std::mutex mtx, m_lck;  // 互斥访问请求队列
    std::condition_variable m_cv;
};

template<class T>
void* ThreadPool<T>::worker(ThreadPool *p) {
    ThreadPool *pool = p;
    pool->run();
    return pool;
}

template<class T>
ThreadPool<T>::ThreadPool(): m_stop(0) {
    m_thread_num = MAX_THREAD_NUM;
    m_max_rqst_num = MAX_RQST_NUM;
    for(int i = 0; i < m_thread_num; i++)
        m_threads.emplace_back(std::thread(worker, this));
}

template<class T>
ThreadPool<T>::~ThreadPool() {
    m_stop = true;
    std::unique_lock<std::mutex> lock(mtx);
    m_cv.notify_all();  // 唤醒所有等待线程，类似的还有notify_one()
    for(auto &t: m_threads)
        t.join();  // 等待线程完成，主线层必须保证所有子线程完成之后才能退出
}

template<class T>
bool ThreadPool<T>::append(T *request, int state) {
    std::unique_lock<std::mutex> lock(mtx);
    if(m_rqst_queue.size() >= m_max_rqst_num)
        return false;
    request->m_state = state;
    m_rqst_queue.push(request);
    lock.unlock();
    m_cv.notify_one();
    return true;
}

template<class T>
void ThreadPool<T>::run() {
    while(!m_stop) {
        std::unique_lock<std::mutex> lock(mtx);
        m_cv.wait(lock, [this] {return !m_rqst_queue.empty();});
        if(m_rqst_queue.empty())
            continue;
        T *request = m_rqst_queue.front();
        m_rqst_queue.pop();
        if(m_act_mode == 1) {
            if(request->m_state == 0) {
                if(request->read_once()) {
                    request->m_improv = 1;
                    request->process();
                }
                else {
                    request->m_improv = 1;
                    request->m_timer_flag = 1;
                }
            }
            else {
                if (request->write()) {
                    request->m_improv = 1;
                }
                else {
                    request->m_improv = 1;
                    request->m_timer_flag = 1;
                }
            }
        }
        else {
            request->process();
        }
    }

}

template<class T>
bool ThreadPool<T>::append_p(T *request) {
    mtx.lock();
    if (m_rqst_queue.size() >= m_max_rqst_num) {
        mtx.unlock();
        return false;
    }
    m_rqst_queue.push(request);
    mtx.unlock();
    m_cv.notify_one();
    return true;
}

template <class T>
int ThreadPool<T>::m_act_mode = 0;

#endif //WEBSERVER_THREADPOOL_H
