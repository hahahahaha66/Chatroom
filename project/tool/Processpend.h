#ifndef PROCESSPEND_H
#define PROCESSPEND_H

#include "../muduo/net/tcp/TcpConnection.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <mutex>
#include <future>
//客户端使用

using json = nlohmann::json;
using Requestcomeback = std::function<bool(const json&, Timestamp)>;
using WrappedHandler = std::function<void(const json&, Timestamp)>;

class Processpend  
{
public:
    //添加受到请求对应的回调函数
    std::future<bool> addRequest(uint16_t seq, Requestcomeback handler)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto promisePtr = std::make_shared<std::promise<bool>>();
        std::future<bool> future = promisePtr->get_future();
        pendingRequests_[seq] = std::move(handler);

        WrappedHandler wrapper = [handler = std::move(handler), promisePtr](const json& js, Timestamp t) mutable {
            try {
                bool result = handler(js, t);
                promisePtr->set_value(result);
            }
            catch (...) {
                // 捕获异常并传给 future
                promisePtr->set_exception(std::current_exception());
            }
        };

        pendingRequests_[seq] = std::move(wrapper);

        return future;
    }

    //通过seq找到对应的回调函数
    void executeRequest(uint16_t seq, const json& js, Timestamp time)
    {
        WrappedHandler cb;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = pendingRequests_.find(seq);
            if (it != pendingRequests_.end())
            {
                cb = std::move(it->second);
                pendingRequests_.erase(seq);
            }
        }

        if (cb)
        {
            cb(js, time);
        }
    }

private:
    std::unordered_map<uint16_t, WrappedHandler> pendingRequests_; 
    std::mutex mutex_;
};

#endif