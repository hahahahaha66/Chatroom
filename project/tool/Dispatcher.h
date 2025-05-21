#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "../muduo/net/tcp/TcpConnection.h"

#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <mutex>
//服务端使用

using json = nlohmann::json;
using MessageHander = std::function<void(const TcpConnectionPtr&, const json&, const uint16_t, Timestamp)>;

class Dispatcher
{
public:
    //注册一个命令对应的回调函数
    void registerHander(uint16_t type, MessageHander hander) {
        std::lock_guard<std::mutex> lock(mutex_);
        handers_[type] = std::move(hander);
    }

    //触发不同命令的回调函数
    void dispatch(uint16_t type, const TcpConnectionPtr conn, const json& js, const uint16_t seq, Timestamp time)
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
            cb(conn, js, seq, time);
        }
    }

private:
    std::unordered_map<uint16_t, MessageHander> handers_;
    std::mutex mutex_;
};

#endif