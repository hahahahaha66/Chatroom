#ifndef DISPATCHER_H
#define DISPATCHER_H

#include "../muduo/logging/Logging.h"
#include "../muduo/net/tcp/TcpConnection.h"
#include "ThreadPool.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;
using MessageHander =
    std::function<void(const TcpConnectionPtr &, const json &)>;

class Dispatcher {
  public:
    Dispatcher(int threadsize = 32) : threadpool_(threadsize) {
        threadpool_.Start();
    }

    ~Dispatcher() { threadpool_.Stop(); }

    // 注册一个命令对应的回调函数
    void registerHandler(std::string type, MessageHander handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        handers_[std::move(type)] = std::move(handler);
    }

    // 触发不同命令的回调函数
    void dispatch(const std::string &type, const TcpConnectionPtr conn,
                  const json &js) {
        MessageHander cb;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = handers_.find(type);
            LOG_DEBUG << type;
            if (it == handers_.end()) {
                LOG_ERROR << "Unrecognized command format";
                return;
            }
            cb = it->second;
            LOG_DEBUG << "hander size: " << sizeof(cb);
        }

        if (!cb) {
            LOG_ERROR << "Callback function is null for type: " << type;
            return;
        }
        auto jsPtr = std::make_shared<json>(js);

        threadpool_.SubmitTask([cb = std::move(cb), type, conn, jsPtr]() {
            LOG_DEBUG << "threadpool push task";
            LOG_DEBUG << "Dispatch type: " << type;
            (cb)(conn, *jsPtr);
        });
    }

  private:
    ThreadPool threadpool_;
    std::unordered_map<std::string, MessageHander> handers_;
    std::mutex mutex_;
};

#endif