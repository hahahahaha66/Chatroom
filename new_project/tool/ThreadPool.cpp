#include "ThreadPool.h"
#include <cstddef>
#include <functional>
#include <mutex>

ThreadPool::ThreadPool(size_t numthread) : running_(false)
{
    workers_.reserve(numthread);
}

ThreadPool::~ThreadPool()
{
    Stop();
}

void ThreadPool::Start()
{
    running_ = true;
    for (size_t i = 0; i < workers_.capacity(); i++ )
    {
        workers_.emplace_back(&ThreadPool::Worker, this);
    }
}

void ThreadPool::Stop()
{
    running_ = false;
    cv_.notify_all();
    for (std::thread &worker : workers_)
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }
}

void ThreadPool::SubmitTask(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.emplace(std::move(task));
    }
    cv_.notify_one();
}

void ThreadPool::Worker()
{
    while (running_)
    {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [&]() { return !tasks_.empty() || !running_; });

            if (!running_ && tasks_.empty()) return;

            task = std::move(tasks_.front());
            tasks_.pop();
        }
        if (task) task();
    }
}