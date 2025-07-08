#ifndef DATABASETHREADPOOL_H
#define DATABASETHREADPOOL_H

#include "MysqlConnection.h"
#include "MysqlConnectionpool.h"
#include "../muduo/logging/Logging.h"

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

struct DatabaseTask 
{
    std::function<void(MysqlConnection&)> task;
    DatabaseTask(std::function<void(MysqlConnection&)> fn) : task(std::move(fn)) {}
};

using MysqlConnPtr = std::shared_ptr<MysqlConnection>;

class DatabaseThreadPool  
{
public:
    DatabaseThreadPool(size_t threadcount = 4);
    ~DatabaseThreadPool();

    void EnqueueTask(std::function<void(MysqlConnection&)> fn);
    void Stop();

private:
    void Worker();

    std::vector<std::thread> workers_;
    std::queue<DatabaseTask> tasks_;
    std::mutex mutex_;
    std::condition_variable con_;
    bool isrunning_;

};

#endif