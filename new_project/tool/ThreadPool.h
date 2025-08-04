#pragma once

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool 
{
public:
    ThreadPool(size_t numthread = 16);
    ~ThreadPool();

    void Start();
    void Stop();
    void SubmitTask(std::function<void()> task);

private:
    void Worker();

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool running_;
};