#ifndef PROCESSPEND_H
#define PROCESSPEND_H

#include "../muduo/net/tcp/TcpConnection.h"

#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <mutex>

using json = nlohmann::json;
using Requestcomeback = std::function<void(const json&, Timestamp)>;

class Processpend  
{
public:
    void addRequest(uint16_t seq, Requestcomeback hander)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        pendingRequests[seq] = std::move(hander);
    }

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