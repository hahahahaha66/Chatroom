#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "../muduo/net/tcp/TcpConnection.h"

#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <mutex>

using json = nlohmann::json;
using MessageHander = std::function<void(const TcpConnectionPtr&, const json&, Timestamp)>;

class Dispatcher
{
public:
    void registerHander(uint16_t type, MessageHander hander) {
        std::lock_guard<std::mutex> lock(mutex_);
        handers_[type] = std::move(hander);
    }

    void dispatch(uint16_t type, const TcpConnectionPtr conn, const json& js, Timestamp time)
    {
        MessageHander cb;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = handers_.find(type);
            if (it != handers_.end())
            {
                cb = std::move(it->second);
            }
            else 
            {
                //添加错误格式处理函数
            }
        }
        
        if (cb)
        {
            cb(conn, js, time);
        }
    }

private:
    std::unordered_map<uint16_t, MessageHander> handers_;
    std::mutex mutex_;
};

#endif