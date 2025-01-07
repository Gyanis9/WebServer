//
// Created by guo on 25-1-5.
//

#include <cassert>
#include <sstream>
#include "../include/EventLoopThreadPool.h"
#include "../include/EventLoopThread.h"
#include "../../base/include/Logger.h"

EventLoopThreadPool::EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg) : m_baseLoop(baseLoop),
                                                                                            m_name(nameArg),
                                                                                            m_started(false),
                                                                                            m_numThreads(0),
                                                                                            m_next(0) {

}

EventLoopThreadPool::~EventLoopThreadPool() = default;

void EventLoopThreadPool::start(const EventLoopThreadPool::ThreadInitCallback &cb) {
    assert(!m_started);
    m_started = true;
    for (int i = 0; i < m_numThreads; ++i) {
        std::vector<char> buf(m_name.size() + 32, 0);
        std::snprintf(buf.data(), buf.size(), "%s%d", m_name.c_str(), i);
        EventLoopThread *thread = new EventLoopThread(cb, buf.data());
        m_threads.emplace_back(thread);
        m_loops.push_back(thread->startLoop());
    }
    if (m_numThreads == 0 && cb) {
        cb(m_baseLoop);
    }
}

EventLoop *EventLoopThreadPool::getNextLoop() {
    assert(m_started);
    EventLoop *loop = m_baseLoop;
    if (!m_loops.empty()) {
        loop = m_loops[m_next];
        ++m_next;
        if (m_next >= m_loops.size()) {
            m_next = 0;
        }
    }
    return loop;
}

std::vector<EventLoop *> EventLoopThreadPool::getAllLoops() {
    assert(m_started);
    if (m_loops.empty()) {
        return std::vector<EventLoop *>(1, m_baseLoop);
    } else {
        return m_loops;
    }
}
