//
// Created by guo on 25-1-5.
//

#ifndef WEBSERVER_EVENTLOOPTHREAD_H
#define WEBSERVER_EVENTLOOPTHREAD_H

#include <mutex>
#include <condition_variable>

#include "../../thread/include/Thread.h"
#include "../../base/include/noncopyable.h"


class EventLoop;

class EventLoopThread : private noncopyable {
public:
    using ThreadInitCallback = std::function<void(EventLoop *)>;
public:
    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string());

    ~EventLoopThread();

    EventLoop *startLoop();


private:
    void threadFunc();

private:
    EventLoop *m_loop;
    bool m_exiting;
    Thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_cond;

    ThreadInitCallback m_callback;
};


#endif //WEBSERVER_EVENTLOOPTHREAD_H
