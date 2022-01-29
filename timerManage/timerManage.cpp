//
// Created by lx on 1/23/22.
//

#include "timerManage.h"

TimerManage::TimerManage(shared_ptr<Utils> utils):m_utils(utils) {}

TimerManage::~TimerManage() {}

void TimerManage::add_timer(Timer *timer) {
    m_min_heap.insertElem(timer);
}

void TimerManage::del_timer(Timer *timer) {
    if (!timer)
        return;

    for(int i = 0; i < m_min_heap.size(); i++) {
        if(m_min_heap[i] == timer) {
            m_min_heap.deleteElem(i);
        }
    }
}

void TimerManage::tick() {
    if(m_min_heap.size() == 0)
        return;

    time_t cur = time(NULL);
    for(int i = 0; i < m_min_heap.size(); i++) {
        Timer *temp = m_min_heap[0];
        if(temp->expire > cur)
            break;
        m_min_heap.popMaxElem();
        temp->cb_func(temp->m_user_data);
    }
}

void TimerManage::sig_handler(int sig) {
    int save_errno = errno;
    int msg = sig;
    send(m_pipefd[1], (char *)&msg, 1, 0);
    errno = save_errno;
}

void TimerManage::timer_handler() {
    tick();
    alarm(m_time_slot);
}

void TimerManage::adjust_timer() {
    m_min_heap.adjust();
}

void TimerManage::addsig(int sig, void (*handler)(int), bool restart) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = handler;
    if (restart)
        sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int *TimerManage::m_pipefd = 0;
int TimerManage::m_time_slot = TIME_SLOT;
