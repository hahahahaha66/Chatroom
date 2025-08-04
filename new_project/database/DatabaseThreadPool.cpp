#include "DatabaseThreadPool.h"
#include "MysqlConnection.h"
#include <cstddef>
#include <functional>
#include <thread>

DatabaseThreadPool::DatabaseThreadPool(size_t threadcount) : isrunning_(true)
{
    for (size_t i = 0; i < threadcount; i++)
    {
        workers_.emplace_back([this, i] () { 
            LOG_INFO << "Worker thread " << i << " started";
            this->Worker(); 
        });
    }
}

DatabaseThreadPool::~DatabaseThreadPool()
{
    Stop();
}

void DatabaseThreadPool::EnqueueTask(std::function<void(MysqlConnection&, DBCallback)> task, DBCallback done) {
    LOG_DEBUG << "one database task add";
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (!isrunning_) {
            LOG_WARN << "DatabaseThreadPool stopped, task rejected";
            // 异步调用回调函数，通知任务失败
            if (done) {
                std::thread([done]() { done(false); }).detach();
            }
            return;
        }
        tasks_.emplace(std::move(task), std::move(done));
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

    LOG_DEBUG << "Database worker thread started";

    while (true)
    {
        DatabaseTask task(nullptr, nullptr);
        {
            std::unique_lock<std::mutex> lock(mutex_);

            //只要任务队列中有任务，或者线程池已经不运行了，就让当前线程从 wait 状态唤醒
            con_.wait(lock, [this]() {return !tasks_.empty() || !isrunning_; });

            if (!isrunning_ && tasks_.empty()) return ;

            if (tasks_.empty()) 
            {
                LOG_DEBUG << "Worker spurious wakeup, continuing";
                continue;
            }
            else  
            {
                task = std::move(tasks_.front());
                tasks_.pop();
                LOG_DEBUG << "Task acquired, queue size: " << tasks_.size();
            }
        }

        MysqlConnPtr conn;
        bool connection_success = false;

        for (int attempt = 0; attempt < 3; ++attempt) 
        {
            conn = MysqlConnectionPool::Instance().GetConnection();
            if (conn) 
            {
                connection_success = true;
                break;
            }
            LOG_WARN << "Failed to get connection (attempt " << (attempt+1) << ")";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (!connection_success) {
            LOG_ERROR << "Giving up after 3 connection attempts";
            //任务失败
            if (task.callback) {
                try {
                    task.callback(false);
                } catch (const std::exception& e) {
                    LOG_ERROR << "Callback execution failed: " << e.what();
                }
            }
            continue;
        }
        else
        {
            std::hash<std::thread::id> hash;
            LOG_DEBUG << "Got connection, thread id: " << hash(std::this_thread::get_id());
        }

        // 执行任务
        try {
            // 执行前验证连接
            if (!conn->IsConnected()) {
                if (!conn->Reconnect()) {
                    throw std::runtime_error("Reconnect failed");
                }
            }
            
            // 执行任务，传入连接和回调函数
            if (task.task) {
                task.task(*conn, task.callback);
            } else if (task.callback) {
                // 如果任务为空但有回调，通知失败
                task.callback(false);
            }
            
        } catch (const std::exception& e) {
            LOG_ERROR << "Database operation failed: " << e.what();
            // 任务执行失败，通知回调
            if (task.callback) {
                try {
                    task.callback(false);
                } catch (const std::exception& cb_e) {
                    LOG_ERROR << "Callback execution failed: " << cb_e.what();
                }
            }
        }
    }
}