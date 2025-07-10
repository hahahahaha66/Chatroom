#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "../muduo/net/tcp/TcpConnection.h"
#include "../muduo/logging/Logging.h"
#include "ThreadPool.h"

#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <mutex>
//服务端使用

using json = nlohmann::json;
using MessageHander = std::function<void(const TcpConnectionPtr&, const json&, Timestamp)>;

class Dispatcher
{
public:
    Dispatcher()
    {
        threadpool_.Start();
    }

    ~Dispatcher()
    {
        threadpool_.Stop();
    }

    //注册一个命令对应的回调函数
    void registerHander(std::string type, MessageHander hander) 
    {
        std::lock_guard<std::mutex> lock(mutex_);
        handers_[type] = std::move(hander);
    }

    //触发不同命令的回调函数
    void dispatch(const std::string type, const TcpConnectionPtr conn, const json& js, Timestamp time)
    {
        MessageHander cb;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = handers_.find(type);
            if (it != handers_.end())
            {
                cb = it->second;
            }
            else 
            {
                LOG_ERROR << "Unrecognized command format";
            }
        }
        
        if (cb)
        {
            threadpool_.SubmitTask([=]() {
                cb(conn, js, time);
            });
        }
    }

private:
    ThreadPool threadpool_;
    std::unordered_map<std::string, MessageHander> handers_;
    std::mutex mutex_;
};

#endif