//
// Created by lx on 1/23/22.
//

#ifndef WEBSERVER_TIMERMANAGE_H
#define WEBSERVER_TIMERMANAGE_H
#include <time.h>
#include <mutex>
#include "../utils/utils.h"
#include "minHeap/minHeap.h"
#include "timer/timer.h"

class Timer;

class TimerManage {
public:
    explicit TimerManage(std::shared_ptr<Utils> utils);
    ~TimerManage();

    void add_timer(Timer *timer);  // 添加定时器
    void del_timer(Timer *timer);  // 删除mer定时器
    void adjust_timer();  // 堆排，调整定时器
    void tick();  // 删除所有超时定时器

    void addsig(int sig, void(handler)(int), bool restart = true);
    static void sig_handler(int sig);


    void timer_handler();  // 定时处理函数，调用tick并继续产生定时器信号

private:
    minHeap<Timer*> m_min_heap;
    shared_ptr<Utils> m_utils;
    static int m_time_slot;
public:
    static int *m_pipefd;  // 用于传输信号, 指向webServer::m_pipefd
};

#endif //WEBSERVER_TIMERMANAGE_H
