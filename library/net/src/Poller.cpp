//
// Created by guo on 25-1-5.
//

#include "../include/Poller.h"
#include "../include/Channel.h"

Poller::Poller(EventLoop *loop) : m_ownerLoop(loop) {

}

bool Poller::hasChannel(Channel *channel) const {
     auto it = m_channels.find(channel->fd());
    return it != m_channels.end() && it->second == channel;
}

Poller::~Poller() = default;


