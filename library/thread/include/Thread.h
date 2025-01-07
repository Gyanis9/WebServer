//
// Created by guo on 24-12-22.
//

#ifndef THREADPOOL_THREAD_H
#define THREADPOOL_THREAD_H

#include <functional>
#include <atomic>
#include <cstdint>
#include <thread>
#include <string>
#include <memory>

class Thread {
public:
    using ThreadFunc = std::function<void()>;
public:
    explicit Thread(ThreadFunc func, const std::string &name = std::string());

    void join();

    ~Thread();

    void start();

    bool started() const { return m_started; }

    const std::string &name() const { return m_name; }

    pid_t tid() const { return m_tid; }


    static std::uint32_t getNumCreated();

    Thread(const Thread &) = delete;

    Thread &operator=(const Thread &) = delete;

private:
    void setDefaultName();

private:
    bool m_started;
    bool m_joined;
    std::string m_name;

    std::shared_ptr<std::thread> m_thread;
    ThreadFunc m_func;
    pid_t m_tid;
    static std::atomic<uint32_t> m_numCreateThread;
};


#endif //THREADPOOL_THREAD_H
