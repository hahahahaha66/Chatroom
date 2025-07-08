#pragma one

#include "../muduo/net/tcp/TcpConnection.h"
#include "../manager/UserManager.h"
#include "../manager/MessageManager.h"
#include "../tool/Codec.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Service {
public:
    Service();

    //User
    void UserRegister(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void UserLogin(const TcpConnectionPtr& conn, const json& json, Timestamp);
    //Message
    void MessageSend(const TcpConnectionPtr& conn, const json& json, Timestamp);
    void GetChatHistory(const TcpConnectionPtr& conn, const json& json, Timestamp);
    //Friend
    //Group
    
private:
    UserManager usermanager_;
    MessageManager messagemanager_;
    Codec code_;
};