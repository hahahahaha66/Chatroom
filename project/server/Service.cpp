#include "Service.h"
#include "../json_protocol.hpp"
#include <cstdint>
#include <functional>

Service::Service()
{
    handermap_[1] = std::bind(&Service::ProcessingLogin, this, _1, _2, _3, _4);

}

void Service::RegisterAllHanders(Dispatcher& dispatcher)
{
    for (auto& [id, hander] : handermap_)
    {
        dispatcher.registerHander(id, hander);
    }
}

void Service::ProcessingLogin(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    std::string username;
    std::string password;
    end &= AssignIfPresent(js, "username", username);
    end &= AssignIfPresent(js, "password", password);

    if (end == true)
    {
        //这里执行从数据库中查询名字及密码是否正确
    }
    
    if (end)
        result = "Login successful!";
    else  
        result = "Login failed!";

    json reply_js = CommandReply(end, result);

    conn->send(codec_.encode(reply_js, 0, seq));
}

void Service::RegisterAccount(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    std::string username;
    std::string password;
    end &= AssignIfPresent(js, "username", username);
    end &= AssignIfPresent(js, "password", password);

    if (end)
    {
        //检查数据库中有无已创建的账号，有返回false，没有则写入返回true
    }

    if (end)
        result = "Register successful!";
    else  
        result = "Register failed!";

    json reply_js = CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListFriendlist(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    std::string username;
    end &= AssignIfPresent(js, "username", username);

    std::vector<std::string> friendlist;
    if (end)
    {
        //读取数据库用户中朋友列表，写入friendlist中,并设置end表示是否成功
    }

    json reply_js = FriendList(username, friendlist);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::DeleteFrient(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    std::string username;
    std::string friendname;

    end &= AssignIfPresent(js, "username", username);
    end &= AssignIfPresent(js, "friendname", friendname);

    if (end)
    {
        //将朋友名字从用户的数据库中删除
    }

    if (end)
        result = "Delete successful!";
    else  
        result = "Delete failed!";

    json reply_js = CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}
