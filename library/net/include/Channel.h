//
// Created by guo on 25-1-3.
//

#ifndef WEBSERVER_CHANNEL_H
#define WEBSERVER_CHANNEL_H

#include "../../base/include/Timestamp.h"
#include "../../base/include/noncopyable.h"

#include <memory>
#include <functional>

class EventLoop;

//Channel理解为通道，封装了 sockfd 及其感兴趣的 event ，如: EPOLLIN、EPOLLOUT
class Channel : private noncopyable {
public:
    using EventCallback = std::function<void()>;//事件回调函数指针
    using ReadEventCallback = std::function<void(Timestamp)>;//读事件回调函数指针
public:
    Channel(EventLoop *loop, int fd);

    ~Channel();

    void handleEvent(Timestamp receiveTime);//事件函数

    void setReadCallback(const ReadEventCallback &cb) { m_readCallback = std::move(cb); }//设置读事件回调函数

    void setWriteCallback(const EventCallback &cb) { m_writeCallback = std::move(cb); }//设置写事件回调函数

    void setCloseCallback(const EventCallback &cb) { m_closeCallback = std::move(cb); }//设置关闭回调函数

    void setErrorCallback(const EventCallback &cb) { m_errorCallback = std::move(cb); }//设置错误回调函数

    void tie(const std::shared_ptr<void> &);//通过智能指针来连接channel的管理,防止当channel被手动remove，channel还在执行回调操作

    int fd() const { return m_fd; }

    int events() const { return m_events; }

    void set_revents(int revt) { m_revents = revt; }//设置事件类型 epoll or poll

    bool isNoneEvent() const { return m_events == kNoneEvent; }


    void enableReading() {//开启读事件
        m_events |= kReadEvent;
        update();
    }

    void disableReading() {//关闭读事件
        m_events &= ~kReadEvent;
        update();
    }

    void enableWriting() {//开启写事件
        m_events |= kWriteEvent;
        update();
    }

    void disableWirting() {//关闭写事件
        m_events &= ~kWriteEvent;
        update();
    }

    void disableAll() {//全部关闭
        m_events = kNoneEvent;
        update();
    }

    bool isWriting() const { return m_events & kWriteEvent; }

    bool isReading() const { return m_events & kReadEvent; }

    int index() const { return m_index; }

    void set_index(int index) { m_index = index; }

    std::string reventToString() const;

    std::string eventToString() const;

    EventLoop *ownerLoop() { return m_loop; }

    void remove();


private:
    static std::string eventsToString(int fd, int ev);

    void update();

    void handleEventWithGuard(Timestamp receiveTime);//根据poller通知channel发生的具体事件，由channel负责调用具体的回调操作

private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

private:
    EventLoop *m_loop;//循环体
    const int m_fd;//当前的fd
    int m_events;//绑定事件
    int m_revents;//事件类型 epoll or poll
    int m_index;

    std::weak_ptr<void> m_tie;
    bool m_tied;

    ReadEventCallback m_readCallback;//读
    EventCallback m_writeCallback;//写
    EventCallback m_closeCallback;//关闭
    EventCallback m_errorCallback;//错误

};


#endif //WEBSERVER_CHANNEL_H
