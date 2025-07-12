#pragma one

#include "../muduo/logging/Logging.h"

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class InitTaskRunner 
{
public:
    static InitTaskRunner& Instance();

    void Start();
    void Stop();

    bool Post(std::function<void()> task);

private:
    InitTaskRunner();
    ~InitTaskRunner();

    InitTaskRunner(const InitTaskRunner&) = delete;
    InitTaskRunner& operator=(const InitTaskRunner&) = delete;

    void WorkerLoop();

    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cond_;
    std::atomic<bool> running_;
    std::thread worker_;

};