//
// Created by guo on 24-12-22.
//

#include "../include/ThreadPool.hpp"
#include <thread>
#include <exception>

ThreadPool::ThreadPool(const std::string &nameArg) : m_name(nameArg), m_maxTaskSize(0), m_isRunning(false) {

}

ThreadPool::~ThreadPool() {
    if (m_isRunning) {
        stop();
    }
}

bool ThreadPool::checkState() const {
    return m_isRunning;
}

void ThreadPool::setTaskMaxSize(uint32_t maxSize) {
    if (checkState())
        std::cerr << "threadPool is running ,can not change!" << std::endl;
    else
        this->m_maxTaskSize = maxSize;
}

void ThreadPool::start(std::uint32_t numThreads) {
    assert(m_threads.empty());
    m_isRunning.store(true);
    m_threads.reserve(numThreads);
    for (std::uint32_t i = 0; i < numThreads; ++i) {
        char id[32];
        snprintf(id, sizeof id, "%d", i + 1);
        auto ptr = std::make_unique<Thread>(std::bind(&ThreadPool::ThreadFun, this), m_name + id);
        m_threads.push_back(std::move(ptr));
        m_threads[i]->start();
    }
    if (numThreads == 0 && m_threadInitCallback) {
        m_threadInitCallback();
    }
}

void ThreadPool::stop() {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_isRunning.store(false);
        m_notEmpty.notify_all();
        m_notFull.notify_all();
    }
    for (auto &thread: m_threads) {
        thread->join();
    }
}

void ThreadPool::run(ThreadPool::Task task) {
    if (m_threads.empty()) {
        task();
    } else {
        std::unique_lock lock(m_mutex);
        m_notFull.wait(lock, [&]() -> bool { return m_isRunning && !isFull(); });
        if (!m_isRunning)
            return;
        assert(!isFull());
        m_tasks.push_back(std::move(task));
        m_notEmpty.notify_one();
    }
}

ThreadPool::Task ThreadPool::take() {
    std::unique_lock lock(m_mutex);
    m_notEmpty.wait(lock, [&]() -> bool { return !m_tasks.empty() || !m_isRunning; });
    Task task;
    if (!m_tasks.empty()) {
        task = std::move(m_tasks.front());
        m_tasks.pop_front();
        if (m_maxTaskSize > 0) {
            m_notFull.notify_all();
        }
    }
    return task;
}


void ThreadPool::ThreadFun() {
    try {
        if (m_threadInitCallback) {
            m_threadInitCallback();
        }
        while (m_isRunning) {
            Task task(take());
            if (!task) { // 如果没有任务且线程池已停止
                break;
            }
            task();
        }
    }
    catch (const std::exception &ex) {
        fprintf(stderr, "exception caught in ThreadPool %s\n", m_name.c_str());
        fprintf(stderr, "reason: %s\n", ex.what());
        abort();
    }
    catch (...) {
        fprintf(stderr, "unknown exception caught in ThreadPool %s\n", m_name.c_str());
        throw;
    }
}

size_t ThreadPool::queueSize() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_tasks.size();
}

bool ThreadPool::isFull() const {
    return m_maxTaskSize > 0 && m_tasks.size() >= m_maxTaskSize;
}










