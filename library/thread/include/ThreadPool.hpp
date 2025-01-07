//
// Created by guo on 24-12-22.
//

#ifndef THREADPOOL_THREADPOOL_HPP
#define THREADPOOL_THREADPOOL_HPP

#include <mutex>
#include <unordered_map>
#include <memory>
#include <functional>
#include <queue>
#include <iostream>
#include <condition_variable>
#include <future>
#include <cassert>
#include "Thread.h"
#include <cstdint>

class ThreadPool {
private:
    using Task = std::function<void()>;
    Task m_threadInitCallback;
public:
    explicit ThreadPool(const std::string &nameArg = std::string("ThreadPool"));

    ~ThreadPool();

    void setTaskMaxSize(uint32_t maxSize);

    size_t queueSize() const;

    void start(uint32_t numThreads = std::thread::hardware_concurrency());

    void stop();

    void run(Task task);

    void setThreadInitCallback(Task cb) { m_threadInitCallback = std::move(cb); }

    Task take();

    template<typename Func, typename ...Args>
    auto submitTask(Func &&func, Args &&...args) -> std::future<typename std::invoke_result<Func, Args...>::type> {
        using Rtype = typename std::invoke_result<Func, Args...>::type;
        auto task = std::make_shared<std::packaged_task<Rtype()>>(
                std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        std::future<Rtype> result(task->get_future());
        std::unique_lock lock(m_mutex);
        m_notFull.wait(lock, [&]() -> bool { return m_isRunning && !isFull(); });
        if (!m_isRunning) {
            throw std::runtime_error("ThreadPool is stopped, cannot submit new tasks.");
        }
        m_tasks.push_back(std::move([task] { (*task)(); }));
        m_notEmpty.notify_one();
        return result;
    }

    ThreadPool(const ThreadPool &) = delete;

    ThreadPool &operator=(const ThreadPool &) = delete;

protected:
    [[nodiscard]]bool checkState() const;

    void ThreadFun();

    bool isFull() const;

private:
    std::vector<std::unique_ptr<Thread>> m_threads;
    std::deque<Task> m_tasks;
    std::atomic<bool> m_isRunning;//线程池运行标志
    size_t m_maxTaskSize;
    mutable std::mutex m_mutex;//线程池互斥量
    std::condition_variable m_notEmpty;
    std::condition_variable m_notFull;
    std::string m_name;
};


#endif //THREADPOOL_THREADPOOL_HPP
