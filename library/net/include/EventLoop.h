//
// Created by guo on 25-1-3.
//

#ifndef WEBSERVER_EVENTLOOP_H
#define WEBSERVER_EVENTLOOP_H

#include "../../base/include/noncopyable.h"
#include "../../base/include/Timestamp.h"
#include "../../thread/include/CurrentThread.h"

#include <functional>
#include <atomic>
#include <memory>
#include <vector>
#include <mutex>

class Channel;

class Poller;

class EventLoop : private noncopyable {
public:
    using Functor = std::function<void()>;
public:
    EventLoop();

    ~EventLoop();

    void loop();

    void quit();

    void runInLoop(Functor cb);

    void queueInLoop(Functor cb);

    size_t queueSize() const;

    void wakeup();

    void updateChannel(Channel *channel);

    void removeChannel(Channel *channel);

    bool hasChannel(Channel *channel);

    bool isInLoopThread() const { return m_threadId == CurrentThread::tid(); }


private:
    using ChannelList = std::vector<Channel *>;

    void handleRead();//waked up

    void doPendingFunctors();


private:
    std::atomic<bool> m_looping;//运行标志
    std::atomic<bool> m_quit;//退出标志
    std::atomic<bool> m_callingPendingFunctors;
    const pid_t m_threadId;     /**记录当前loop所在线程的id*/
    Timestamp m_pollReturnTime;
    std::unique_ptr<Poller> m_poller;
    int m_wakefd;
    std::unique_ptr<Channel> m_wakeupChannel;

    ChannelList m_activeChannels;
    Channel *m_currentActiveChannel;

    mutable std::mutex m_mutex;
    std::vector<Functor> m_pendingFunctors;
};


#endif //WEBSERVER_EVENTLOOP_H
