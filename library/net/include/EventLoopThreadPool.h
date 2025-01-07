//
// Created by guo on 25-1-5.
//

#ifndef WEBSERVER_EVENTLOOPTHREADPOOL_H
#define WEBSERVER_EVENTLOOPTHREADPOOL_H

#include "../../base/include/noncopyable.h"

#include <functional>
#include <memory>
#include <vector>
#include <string>

class EventLoop;

class EventLoopThread;


class EventLoopThreadPool : private noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
public:
    EventLoopThreadPool(EventLoop *baseLoop, const std::string &nameArg);

    ~EventLoopThreadPool();

    void setThreadNum(int numThreads) { m_numThreads = numThreads; }

    void start(const ThreadInitCallback &cb = ThreadInitCallback());

    EventLoop *getNextLoop();

    std::vector<EventLoop *> getAllLoops();

    bool started() const { return m_started; }

    const std::string &name() const { return m_name; }

private:
    EventLoop *m_baseLoop;
    std::string m_name;
    bool m_started;
    int m_numThreads;
    int m_next;
    std::vector<std::unique_ptr<EventLoopThread>> m_threads;
    std::vector<EventLoop *> m_loops;

};


#endif //WEBSERVER_EVENTLOOPTHREADPOOL_H
