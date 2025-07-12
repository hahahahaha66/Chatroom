#include "InitTaskRunner.h"

InitTaskRunner::InitTaskRunner() : running_(false) {}

InitTaskRunner::~InitTaskRunner()
{
    Stop();
}

InitTaskRunner& InitTaskRunner::Instance()
{
    static InitTaskRunner instance;
    return instance;
}

void InitTaskRunner::Start()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_ == true) return;
    LOG_INFO << "单线程任务执行器开始";

    running_.store(true);
    worker_ = std::thread(&InitTaskRunner::WorkerLoop, this);
}

void InitTaskRunner::Stop()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_.load()) {
            return;
        }
        running_.store(false);
    }

    cond_.notify_one();

    if (worker_.joinable())
    {
        worker_.join();
    }
}

bool InitTaskRunner::Post(std::function<void()> task)
{
    if (!task || !running_.load()) 
    {
            return false;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!running_.load()) 
        {
                return false;
        }
        tasks_.push(task);
    }

    LOG_INFO << "任务推送,队列大小为" << tasks_.size();
    cond_.notify_one();
    return true;
}

void InitTaskRunner::WorkerLoop()
{
    while (true)
    {
        std::function<void()> task;

        {
            std::unique_lock<std::mutex> lock(mutex_);

            cond_.wait(lock, [this]() {
                return !tasks_.empty() || !running_;
            });

            if (!running_ && tasks_.empty()) break;

            task = std::move(tasks_.front());
            tasks_.pop();
        }

        try {
            LOG_INFO << "执行系统初始化及数据库刷新任务";
            if (task) task();
        }
        catch (const std::exception& e) {
            LOG_ERROR << "InitTaskRunner 执行任务时捕获异常: " << e.what();
        }
        catch (...) {
            LOG_ERROR << "InitTaskRunner 执行任务时捕获未知异常";
        }

        LOG_INFO << "单线程任务执行器退出";
    }
}