//
// Created by guo on 24-12-22.
//

#include <cstdio>
#include <cassert>
#include <semaphore.h>

#include "../include/Thread.h"
#include "../include/CurrentThread.h"


std::atomic<uint32_t> Thread::m_numCreateThread(0);

Thread::Thread(Thread::ThreadFunc func, const std::string &name) : m_started(false), m_joined(false),
                                                                   m_func(std::move(func)), m_tid(0),
                                                                   m_name(name) {
    setDefaultName();
}

void Thread::start() {
    assert(!m_started);
    m_started = true;
    sem_t sem;
    sem_init(&sem, false, 0);
    m_thread = std::shared_ptr<std::thread>(new std::thread([&]() {
        m_tid = CurrentThread::tid();
        sem_post(&sem);
        m_func();
    }));
    sem_wait(&sem);
}

uint32_t Thread::getNumCreated() {
    return Thread::m_numCreateThread.load();
}

Thread::~Thread() {
    if ((m_started && !m_joined))
        m_thread->detach();
}

void Thread::join() {
    assert(m_started);
    assert(!m_joined);
    m_joined = true;
    if (m_thread->joinable()) {
        m_thread->join();
    }
}

void Thread::setDefaultName() {
    auto num = ++m_numCreateThread;
    if (m_name.empty()) {
        char buf[32];
        snprintf(buf, sizeof buf, "Thread%d", num);
        m_name = buf;
    }
}


