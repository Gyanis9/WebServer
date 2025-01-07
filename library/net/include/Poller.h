//
// Created by guo on 25-1-5.
//

#ifndef WEBSERVER_POLLER_H
#define WEBSERVER_POLLER_H

#include <vector>
#include <unordered_map>


#include "../../base/include/Timestamp.h"
#include "../../base/include/noncopyable.h"
#include "EventLoop.h"

class Channel;

//多路事件分发器的核心IO复用模块
class Poller : private noncopyable {
public:
    using ChannelList = std::vector<Channel *>;
public:
    Poller(EventLoop *loop);

    virtual ~Poller();

    virtual Timestamp poll(int timeoutMs, ChannelList *activateChannels) = 0;

    virtual void updateChannel(Channel *channel) = 0;

    virtual void removeChannel(Channel *channel) = 0;

    virtual bool hasChannel(Channel *channel) const;

    static Poller *newDefaultPoller(EventLoop *loop);

protected:
    using ChannelMap = std::unordered_map<int, Channel *>;
    ChannelMap m_channels;
private:
    EventLoop *m_ownerLoop;
};


#endif //WEBSERVER_POLLER_H
