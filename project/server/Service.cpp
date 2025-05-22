#include "Service.h"
#include "../json_protocol.hpp"
#include <cstdint>
#include <functional>
#include <unordered_set>

Service::Service(Dispatcher& dispatcher) : dispatcher_(dispatcher)
{
    handermap_[1] = std::bind(&Service::ProcessingLogin, this, _1, _2, _3, _4);
    handermap_[2] = std::bind(&Service::RegisterAccount, this, _1, _2, _3, _4);
    handermap_[5] = std::bind(&Service::ListFriendlist, this, _1, _2, _3, _4);
    handermap_[7] = std::bind(&Service::DeleteFrient, this, _1, _2, _3, _4);
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

    int userid;
    std::string password;
    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "password", password);

    if (end == true)
    {
        //这里执行从数据库中查询名字及密码是否正确
    }
    
    if (end)
        result = "Login successful!";
    else  
        result = "Login failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);
    if (end) 
        userlist_[userid].SetOnline(conn);

    conn->send(codec_.encode(reply_js, 0, seq));
}

void Service::RegisterAccount(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    std::string username;
    std::string password;
    end &= AssignIfPresent(js, "userid", userid);
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

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);
    if (end)
    {
        userlist_[userid] = {userid, username, password};
    }
       
    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::ListFriendlist(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;

    int userid;
    end &= AssignIfPresent(js, "userid", userid);

    std::unordered_set<int> friendset = userlist_[userid].GetFriendList();
    std::vector<int> friendlist;
    for (auto& it : friendset)
    {
        friendlist.push_back(it);
    }

    json reply_js = js_FriendList(userid, friendlist);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}

void Service::DeleteFrient(const TcpConnectionPtr& conn, const json& js, const uint16_t seq, Timestamp time)
{
    bool end = true;
    std::string result;

    int userid;
    int friendid;

    end &= AssignIfPresent(js, "userid", userid);
    end &= AssignIfPresent(js, "friendid", friendid);

    userlist_[userid].DeleteFriend(friendid);

    if (end)
        result = "Delete successful!";
    else  
        result = "Delete failed!";

    json reply_js = js_CommandReply(end, result);
    uint16_t type = (end == true ? 1 : 0);

    conn->send(codec_.encode(reply_js, type, seq));
}
