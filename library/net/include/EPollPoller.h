//
// Created by guo on 25-1-5.
//

#ifndef WEBSERVER_EPOLLPOLLER_H
#define WEBSERVER_EPOLLPOLLER_H

#include "Poller.h"

struct epoll_event;

class EPollPoller : public Poller {
public:
    explicit EPollPoller(EventLoop *loop);

    ~EPollPoller() override;

    Timestamp poll(int timeoutMs, ChannelList *actiavteChannles) override;

    void updateChannel(Channel *channel) override;

    void removeChannel(Channel *channel) override;

private:
    static const int kInitEventListSize = 16;

    static const char *operationToString(int op);

    //为activeChannels内填充需要响应操作的事件
    void fillActivateChannels(int numEvents, ChannelList *activateChannels) const;

    void update(int operation, Channel *channel);

private:
    using EventList = std::vector<epoll_event>;

    int m_epollfd;
    EventList m_events;//事件集合
};


#endif //WEBSERVER_EPOLLPOLLER_H
