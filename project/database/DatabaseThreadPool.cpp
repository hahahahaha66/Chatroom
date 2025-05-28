#include "DatabaseThreadPool.h"

DatabaseThreadPool::DatabaseThreadPool(size_t threadcount) : isrunning_(true)
{
    for (int i = 0; i < threadcount; i++)
    {
        workers_.emplace_back([this] () { this->Worker(); });
    }
}

DatabaseThreadPool::~DatabaseThreadPool()
{
    Stop();
}

void DatabaseThreadPool::EnqueueTask(std::function<void(MysqlConnection&)> fn)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        tasks_.push(std::move(fn));
    }
    con_.notify_one();
}

void DatabaseThreadPool::Stop()
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        isrunning_ = false;
    }
    con_.notify_all();
    for (auto& it : workers_)
    {
        if (it.joinable())
            it.join();
    }
}

void DatabaseThreadPool::Worker()
{
    while (true)
    {
        DatabaseTask task(nullptr);
        {
            std::unique_lock<std::mutex> lock(mutex_);
            //只要任务队列中有任务，或者线程池已经不运行了，就让当前线程从 wait 状态唤醒
            con_.wait(lock, [this]() {return !tasks_.empty() || !isrunning_; });
            if (!isrunning_ && tasks_.empty()) return ;
            task = std::move(tasks_.front());
            tasks_.pop();
        }

        auto conn = MysqlConnectionPool::Instance().GetConnection();
        if (conn && task.task)
        {
            task.task(*conn);
        }
    }
}