//
// Created by guo on 25-1-5.
//

#include <cassert>
#include "../include/EventLoopThread.h"
#include "../include/EventLoop.h"

EventLoopThread::EventLoopThread(const EventLoopThread::ThreadInitCallback &cb, const std::string &name) : m_loop(
        nullptr), m_exiting(false), m_thread(std::bind(&EventLoopThread::threadFunc, this), name), m_mutex(), m_cond(),
                                                                                                           m_callback(cb) {

}

EventLoopThread::~EventLoopThread() {
    m_exiting = true;
    if (m_loop != nullptr) {
        m_loop->quit();
        m_thread.join();
    }
}

EventLoop *EventLoopThread::startLoop() {
    assert(!m_thread.started());
    m_thread.start();
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(m_mutex);;
        m_cond.wait(lock, [&]() -> bool { return m_loop != nullptr; });
        loop = m_loop;
    }
    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;
    if (m_callback) {
        m_callback(&loop);
    }
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_loop = &loop;
        m_cond.notify_one();
    }
    loop.loop();
    std::lock_guard<std::mutex> lock(m_mutex);
    m_loop = nullptr;
}
