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

using DBCallback = std::function<void(bool)>;
using MysqlConnPtr = std::shared_ptr<MysqlConnection>;

struct DatabaseTask {
    std::function<void(MysqlConnection&, DBCallback)> task;
    DBCallback callback;
    
    DatabaseTask() : task(nullptr), callback(nullptr) {}
    DatabaseTask(std::function<void(MysqlConnection&, DBCallback)> t, DBCallback cb) 
        : task(std::move(t)), callback(std::move(cb)) {}
};

class DatabaseThreadPool  
{
public:
    DatabaseThreadPool(size_t threadcount = 10);
    ~DatabaseThreadPool();

    void EnqueueTask(std::function<void(MysqlConnection&, DBCallback)> task, DBCallback done);
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