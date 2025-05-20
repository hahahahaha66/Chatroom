#include "../muduo/net/tcp/TcpConnection.h"

#include <cstdint>
#include <functional>
#include <nlohmann/json.hpp>
#include <unordered_map>

using json = nlohmann::json;
using MessageHander = std::function<void(const TcpConnectionPtr&, const json&, Timestamp)>;

class Dispatcher
{
public:
    void registerHander(uint16_t type, MessageHander hander) {
        handers_[type] = std::move(hander);
    }

    void dispatch(uint16_t type, const TcpConnectionPtr conn, const json& js, Timestamp time)
    {
        auto it = handers_.find(type);
        if (it != handers_.end())
        {
            it->second(conn, js, time);
        }
        else 
        {

        }
    }

private:
    std::unordered_map<uint16_t, MessageHander> handers_;
};