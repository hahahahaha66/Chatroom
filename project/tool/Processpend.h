#ifndef PROCESSPEND_H
#define PROCESSPEND_H

#include "../muduo/net/tcp/TcpConnection.h"

#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <mutex>
//客户端使用

using json = nlohmann::json;
using Requestcomeback = std::function<void(const json&, Timestamp)>;

class Processpend  
{
public:
    //添加受到请求对应的回调函数
    void addRequest(uint16_t seq, Requestcomeback hander)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingRequests[seq] = std::move(hander);
    }

    //通过seq找到对应的回调函数
    void executeRequest(uint16_t seq, const json& js, Timestamp time)
    {
        Requestcomeback cb;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = pendingRequests.find(seq);
            if (it != pendingRequests.end())
            {
                cb = std::move(it->second);
                pendingRequests.erase(seq);
            }
        }

        if (cb)
        {
            cb(js, time);
        }
    }

private:
    std::unordered_map<uint16_t, Requestcomeback> pendingRequests; 
    std::mutex mutex_;
};

#endif