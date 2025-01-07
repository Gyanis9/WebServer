//
// Created by guo on 25-1-5.
//


#include <sys/epoll.h>
#include <cassert>
#include <cstring>


#include "../include/EPollPoller.h"
#include "../include/Channel.h"
#include "../../base/include/Logger.h"


const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;


EPollPoller::EPollPoller(EventLoop *loop) : Poller(loop),
                                            m_epollfd(::epoll_create1(EPOLL_CLOEXEC)),
                                            m_events(kInitEventListSize) {
    if (m_epollfd < 0) {
        LOG_SYSFATAL << "EPollPoller::EPollPoller";
    }
}

EPollPoller::~EPollPoller() {
    ::close(m_epollfd);
}

Timestamp EPollPoller::poll(int timeoutMs, Poller::ChannelList *actiavteChannles) {
    LOG_TRACE << "fd total count " << m_channels.size();
    int numEvents = ::epoll_wait(m_epollfd, &*m_events.begin(), static_cast<int>(m_events.size()), timeoutMs);
    int saveError = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happened";
        fillActivateChannels(numEvents, actiavteChannles);
        if (numEvents == m_events.size()) {//如果填满这说明空间不足需要扩充
            m_events.resize(m_events.size() * 2);
        }
    } else if (numEvents == 0) {
        LOG_TRACE << " nothing happened";
    } else {
        if (saveError != EINTR) {
            errno = saveError;
            LOG_SYSERR << "EPollPoller::poll()";
        }
    }
    return now;
}

void EPollPoller::updateChannel(Channel *channel) {
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events() << " index = " << index;
    if (index == kNew || index == kDeleted) {
        int fd = channel->fd();
        if (index == kNew) {
            assert(m_channels.find(fd) == m_channels.end());
            m_channels[fd] = channel;
        } else {
            assert(m_channels.find(fd) != m_channels.end());
            assert(m_channels[fd] == channel);
        }
        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        int fd = channel->fd();
        assert(m_channels.find(fd) != m_channels.end());
        assert(m_channels[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel *channel) {
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(m_channels.find(fd) != m_channels.end());
    assert(m_channels[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    m_channels.erase(fd);
    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

const char *EPollPoller::operationToString(int op) {
    switch (op) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "UnKnown Operation";
    }
}

void EPollPoller::fillActivateChannels(int numEvents, Poller::ChannelList *activateChannels) const {
    assert(numEvents <= m_events.size());
    for (int i = 0; i < numEvents; ++i) {
        Channel *channel = static_cast<Channel *>(m_events[i].data.ptr);
        channel->set_revents(m_events[i].events);
        activateChannels->push_back(channel);
    }
}

void EPollPoller::update(int operation, Channel *channel) {
    epoll_event event;
    bzero(&event, sizeof event);
    event.events = channel->events();
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << operationToString(operation) << " fd = " << fd << " event = {"
              << channel->eventToString() << "}";
    if (::epoll_ctl(m_epollfd, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_SYSERR << "epoll_ctl op = " << operationToString(operation) << " fd = " << fd;
        } else {
            LOG_SYSFATAL << "epoll_ctl op = " << operationToString(operation) << " fd = " << fd;
        }
    }
}
