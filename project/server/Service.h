#ifndef SERVICE_H
#define SERVICE_H

#include "../muduo/net/tcp/TcpConnection.h"
#include "../tool/Dispatcher.h"
#include "../tool/Codec.h"

#include <cstdint>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Service
{
public:
    Service();
    ~Service();

    void ProcessingLogin(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time);

private:
    Dispatcher dispatcher_;
    Codec codec_;
};

#endif