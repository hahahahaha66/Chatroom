#include "MysqlConnectionpool.h"
#include "MysqlConnection.h"

#include <cstddef>
#include <chrono>
#include <memory>
#include <mutex>
#include <thread>

MysqlConnectionPool::MysqlConnectionPool() : isrunning_(true) {}

MysqlConnectionPool::~MysqlConnectionPool()
{
    isrunning_ = false;
    cond_.notify_all();
    while (!connectionqueue_.empty())
    {
        auto conn = connectionqueue_.front();
        connectionqueue_.pop();
        delete conn;
    }
}

MysqlConnectionPool& MysqlConnectionPool::Instance()
{
    static MysqlConnectionPool instance;
    return instance;
}

void MysqlConnectionPool::Init(const std::string& host, unsigned short port, const std::string& user,
                               const std::string& password, const std::string& name, int maxsize)
{
    host_ = host;
    port_ = port;
    user_ = user;
    password_ = password;
    name_ = name;
    maxsize_ = maxsize;

    for (int i = 0; i < maxsize_ / 2; i++)
    {
        auto conn = new MysqlConnection;
        if(conn->Connect(host_, port_, user_, password_, name))
        {
            conn->RefrushAliveTime();
            connectionqueue_.push(conn);
        }
        else  
        {
            LOG_ERROR << "Failed to create initial connection to database";
            delete conn;
        }
    }

    std::thread(&MysqlConnectionPool::ProduceConnectionTask, this).detach();
    std::thread(&MysqlConnectionPool::RecycleMysqlConnectionTask, this).detach();
}

std::shared_ptr<MysqlConnection> MysqlConnectionPool::GetConnection()
{
    MysqlConnection* conn = nullptr;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (connectionqueue_.empty()) 
        {
            if (cond_.wait_for(lock, std::chrono::seconds(1)) == std::cv_status::timeout) 
            {
                LOG_ERROR << "Timeout waiting for connection";
                return nullptr;
            }
        }

        conn = connectionqueue_.front();
        connectionqueue_.pop();
    }

    if (!conn->IsConnected() && !conn->Reconnect()) {
        delete conn;
        return nullptr; 
    }

    std::shared_ptr<MysqlConnection> sp(conn, [this](MysqlConnection* ptr)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        ptr->RefrushAliveTime();
        connectionqueue_.push(ptr);
        cond_.notify_one();
    });

    return sp;
}

void MysqlConnectionPool::ProduceConnectionTask()
{
    while (isrunning_)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        while (connectionqueue_.size() >= static_cast<size_t>(maxsize_))
        {
            if (!isrunning_) return;
            cond_.wait_for(lock, std::chrono::seconds(1));
        }

        size_t total_connections = connectionqueue_.size();
        size_t new_connections = std::min(
            static_cast<size_t>(maxsize_) - total_connections,
            static_cast<size_t>(maxsize_ / 2)
        );

        for (size_t i = 0; i < new_connections; ++i) {
            auto conn = new MysqlConnection;
            if (conn->Connect(host_, port_, user_, password_, name_)) {
                conn->RefrushAliveTime();
                connectionqueue_.push(conn);
                cond_.notify_one(); 
            } else {
                delete conn;
            }
        }
    }
}

void MysqlConnectionPool::RecycleMysqlConnectionTask()
{
    while (isrunning_)
    {
        std::this_thread::sleep_for(std::chrono::seconds(60));

        std::lock_guard<std::mutex> lock(mutex_);
        size_t size = connectionqueue_.size();
        while (size--)
        {
            auto conn = connectionqueue_.front();
            connectionqueue_.pop();

            if (conn->GetLastTime() >= 60 * 10 * 1000)
            {
                delete conn;
            }
            else  
            {
                connectionqueue_.push(conn);
            }
        }
    }
}