#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "../muduo/net/tcp/TcpConnection.h"
#include "../muduo/logging/Logging.h"
#include "ThreadPool.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <mutex>
//服务端使用

using json = nlohmann::json;
using MessageHander = std::function<void(const TcpConnectionPtr&, const json&)>;

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
    void registerHandler(std::string type, MessageHander handler) 
    {
        std::lock_guard<std::mutex> lock(mutex_);
        handers_[type] = std::make_shared<MessageHander>(handler);
    }

    //触发不同命令的回调函数
    void dispatch(const std::string type, const TcpConnectionPtr conn, const json& js)
    {
        std::shared_ptr<MessageHander> cb;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = handers_.find(type);
            LOG_DEBUG << type;
            if (it == handers_.end())
            {
                LOG_ERROR << "Unrecognized command format";
                return;
            }
            cb = it->second;
            LOG_DEBUG << "hander size: " << sizeof(cb);
        }
        threadpool_.SubmitTask([cb, type, conn, js = std::move(js)]() {
            LOG_DEBUG << "threadpool push task";
            LOG_DEBUG << "Decode message : type = " << type << ", json = " << js.dump();
            (*cb)(conn, js);
        });
    }

private:
    ThreadPool threadpool_;
    std::unordered_map<std::string, std::shared_ptr<MessageHander>> handers_;
    std::mutex mutex_;
};

#endif