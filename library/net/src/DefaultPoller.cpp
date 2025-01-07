//
// Created by guo on 25-1-5.
//
#include "../include/Poller.h"
#include "../include/EPollPoller.h"

Poller *Poller::newDefaultPoller(EventLoop *loop) {
    return new EPollPoller(loop);
}