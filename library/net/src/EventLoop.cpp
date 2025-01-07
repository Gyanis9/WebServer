//
// Created by guo on 25-1-3.
//
#include <signal.h>
#include <sys/eventfd.h>
#include <thread>
#include <cassert>


#include "../include/EventLoop.h"
#include "../../base/include/Logger.h"

#include "../include/Channel.h"
#include "../include/Poller.h"

const int kPollTimeMs = 10000;//定义默认的Poller IO复用接口的超时时间 10s

thread_local EventLoop *t_loopInThisThread = nullptr;

int createEventFd() {
    int evtfd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (evtfd < 0) {
        LOG_SYSERR << "Failed in eventfd";
        abort();
    }
    return evtfd;
}

EventLoop::EventLoop() : m_looping(false),
                         m_quit(false),
                         m_callingPendingFunctors(false),
                         m_threadId(CurrentThread::tid()),
                         m_poller(Poller::newDefaultPoller(this)),
                         m_wakefd(createEventFd()),
                         m_wakeupChannel(new Channel(this, m_wakefd)),
                         m_currentActiveChannel(nullptr) {
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop %p exists int this Thread ";
    } else {
        t_loopInThisThread = this;
    }
    LOG_DEBUG << "EventLoop created " << this << " in thread " << std::this_thread::get_id();
    m_wakeupChannel->setReadCallback(std::bind(&EventLoop::handleRead, this));
    m_wakeupChannel->enableReading();
}

EventLoop::~EventLoop() {
    LOG_DEBUG << "EventLoop " << this << " of thread " << std::this_thread::get_id() << " destructs int thread ";
    m_wakeupChannel->disableAll();
    m_wakeupChannel->remove();
    ::close(m_wakefd);
    t_loopInThisThread = nullptr;
}

void EventLoop::loop() {
    assert(!m_looping);
    m_looping = true;
    m_quit = false;
    LOG_TRACE << "EventLoop " << this << " start looping";
    while (!m_quit) {
        m_activeChannels.clear();
        m_pollReturnTime = m_poller->poll(kPollTimeMs, &m_activeChannels);
        for (Channel *channel: m_activeChannels) {
            m_currentActiveChannel = channel;
            m_currentActiveChannel->handleEvent(m_pollReturnTime);
        }
        m_currentActiveChannel = nullptr;
        doPendingFunctors();
    }
    LOG_TRACE << "EventLoop " << this << " stop looping";
    m_looping = false;
}

void EventLoop::quit() {
    m_quit = true;
    if (!isInLoopThread()) {
        wakeup();
    }
}

void EventLoop::runInLoop(EventLoop::Functor cb) {
    if (isInLoopThread()) {
        cb();
    } else {
        queueInLoop(std::move(cb));
    }
}

void EventLoop::queueInLoop(EventLoop::Functor cb) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_pendingFunctors.emplace_back(cb);
    }
    if (isInLoopThread() || m_callingPendingFunctors) {
        wakeup();
    }
}

size_t EventLoop::queueSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_pendingFunctors.size();

}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(m_wakefd, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::wakeup() writes" << n << "bytes instead of 8";
    }
}

void EventLoop::updateChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    m_poller->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    m_poller->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    assert(channel->ownerLoop() == this);
    return m_poller->hasChannel(channel);
}

void EventLoop::handleRead() {
    uint64_t one = 1;
    ssize_t n = read(m_wakefd, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << "bytes instead of 8";
    }
}

void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    m_callingPendingFunctors = true;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        functors.swap(m_pendingFunctors);
    }
    for (const Functor &func: functors) {
        func();
    }
    m_callingPendingFunctors = false;
}



