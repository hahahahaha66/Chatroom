#include "Service.h"
#include "../json_protocol.hpp"
#include <cstdint>

Service::Service()
{
    dispatcher_.registerHander(1, [this](const TcpConnectionPtr& conn, const json& js, const uint16_t seq,Timestamp time)
    { this->ProcessingLogin(conn, js, seq, time); });

}

void Service::ProcessingLogin(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    std::string name = ExtractCommonField(js, "name");
    std::string password = ExtractCommonField(js, "password");

    bool end = true;
    std::string result;
    //这里执行从数据库中查询名字及密码是否正确
    if (end)
        result = "Login successful!";
    else  
        result = "Login failed!";

    json reply_js = CommandReply(end, result);

    conn->send(codec_.encode(reply_js, 0, seq));

}

