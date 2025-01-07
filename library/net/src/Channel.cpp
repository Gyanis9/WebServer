//
// Created by guo on 25-1-3.
//

#include "../include/Channel.h"
#include "../../base/include/Logger.h"
#include "../include/EventLoop.h"

#include <sys/epoll.h>
#include <cassert>
#include <sstream>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop *loop, int fd) :
        m_loop(loop),
        m_fd(fd), m_events(0),
        m_revents(0),
        m_index(-1),
        m_tied(false) {

}

Channel::~Channel() = default;

void Channel::handleEvent(Timestamp receiveTime) {
    //创建一个空的 shared_ptr，用于保持对象的生命周期
    std::shared_ptr<void> guard = nullptr;
    if (m_tied) {//检查当前 Channel 是否被绑定（tied）
        guard = m_tie.lock();//尝试从弱引用 m_tie 中获取 shared_ptr
        if (guard) {
            handleEventWithGuard(receiveTime);
        }
    } else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::tie(const std::shared_ptr<void> &obj) {
    m_tie = obj;
    m_tied = true;
}

std::string Channel::reventToString() const {
    return eventsToString(m_fd, m_revents);
}

std::string Channel::eventToString() const {
    return eventsToString(m_fd, m_events);
}

std::string Channel::eventsToString(int fd, int ev) {
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & EPOLLIN)
        oss << "IN ";
    if (ev & EPOLLPRI)
        oss << "PRI ";
    if (ev & EPOLLOUT)
        oss << "OUT ";
    if (ev & EPOLLHUP)
        oss << "HUB ";
    if (ev & EPOLLRDHUP)
        oss << "RDHUP ";
    if (ev & EPOLLERR)
        oss << "ERR ";
    return oss.str();
}

void Channel::remove() {
    assert(isNoneEvent());
    m_loop->removeChannel(this);
}

void Channel::update() {
    m_loop->updateChannel(this);
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
    LOG_TRACE << "Channel::handleEventWithGuard called for fd " << m_fd << ", events: " << reventToString();
    if ((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN)) {
        if (m_closeCallback)
            m_closeCallback();
    }
    if (m_revents & EPOLLERR) {
        if (m_errorCallback)
            m_errorCallback();
    }
    if (m_revents & (EPOLLIN | EPOLLPRI)) {
        if (m_readCallback)
            m_readCallback(receiveTime);
    }
    if (m_revents & EPOLLOUT) {
        if (m_writeCallback)
            m_writeCallback();
    }
}

