#include "Client.h"

//提取json变量
template<typename  T>
std::optional<T> ExtractCommonField(const json& j, const std::string& key)
{
    try 
    {
        if (j.contains(key) && !j.at(key).is_null())
        {
            return j.at(key).get<T>();
        }
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << "Extract field [" << key << "] error: " << e.what();
        // 也可以记录当前的json值
        LOG_ERROR << "Json value for key [" << key << "] is: " << j.at(key).dump();
    }
    return std::nullopt;
}

//更完备的提取json变量
template<typename T>
bool AssignIfPresent(const json& j, const std::string& key, T& out)
{
    if (j.is_string())
    {
        try 
        {
            json pared = json::parse(j.get<std::string>());
            return AssignIfPresent(pared, key, out);
        }
        catch(...)
        {
            return false;
        }
    }

    auto opt = ExtractCommonField<T>(j, key);
    if (opt.has_value()) {
        out = std::move(opt.value());
        return true;
    }
    return false;
}

std::string Gettime()
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::string timestamp_str = std::ctime(&time_t);
    // 移除换行符
    if (!timestamp_str.empty() && timestamp_str.back() == '\n') {
        timestamp_str.pop_back();
    }
    return timestamp_str;
}

bool ReadNum(std::string input, int &result)
{
    int value;
    std::stringstream ss(input);
     
    if (ss >> value && ss.eof())
    {
        result = value;
        return true;
    }
    else  
    {
        std::cout << std::endl << "格式输入无效,请重试" << std::endl;
        return false;
    }
}

void ClearScreen() 
{
    std::cout << "\033[2J\033[H";
}

Client::Client(EventLoop& loop, InetAddress addr, std::string name)
    : client_(&loop, addr, name)
{
    client_.setConnectionCallback(std::bind(&Client::ConnectionCallBack, this, _1));
    client_.setMessageCallback(std::bind(&Client::OnMessage, this, _1, _2, _3));
    client_.setWriteCompleteCallback(std::bind(&Client::MessageCompleteCallback, this, _1));

    dispatcher_.registerHander("RegisterBack", std::bind(&Client::RegisterBack, this, _1, _2));
    dispatcher_.registerHander("LoginBack", std::bind(&Client::LoginBack, this, _1, _2));
    dispatcher_.registerHander("SendMessageBack", std::bind(&Client::SendMessageBack, this, _1, _2));
    dispatcher_.registerHander("RecvMessage", std::bind(&Client::RecvMessageBack, this, _1, _2));
    dispatcher_.registerHander("GetChatHistoryBack", std::bind(&Client::GetChatHistoryBack, this, _1, _2));
    dispatcher_.registerHander("GetGroupHistoryBack", std::bind(&Client::GetGroupHistoryBack, this, _1, _2));
    dispatcher_.registerHander("SendFriendApplyBack", std::bind(&Client::SendFriendApplyBack, this, _1, _2));
    dispatcher_.registerHander("ListFriendApplyBack", std::bind(&Client::ListFriendApplyBack, this, _1, _2));
    dispatcher_.registerHander("ListSendFriendApplyBack", std::bind(&Client::ListSendFriendApplyBack, this, _1, _2));
    dispatcher_.registerHander("ProceFriendApplyBack", std::bind(&Client::ProceFriendApplyBack, this, _1, _2));
    dispatcher_.registerHander("ListFriendBack", std::bind(&Client::ListFriendBack, this, _1, _2));
    dispatcher_.registerHander("CreateGroupBack", std::bind(&Client::CreateGroupBack, this, _1, _2));
    dispatcher_.registerHander("SendGroupApplyBack", std::bind(&Client::SendGroupApplyBack, this, _1, _2));
    dispatcher_.registerHander("ListGroupApplyBack", std::bind(&Client::ListGroupApplyBack, this, _1, _2));
    dispatcher_.registerHander("ListSendGroupApplyBack", std::bind(&Client::ListSendGroupApplyBack, this, _1, _2));
    dispatcher_.registerHander("ListGroupMemberBack", std::bind(&Client::GroupMemberBack, this, _1, _2));
    dispatcher_.registerHander("ListGroupBack", std::bind(&Client::ListGroupBack, this, _1, _2));
    dispatcher_.registerHander("QuitGrouprBack", std::bind(&Client::QuitGroupBack, this, _1, _2));
    dispatcher_.registerHander("ProceGroupApplyBack", std::bind(&Client::ProceGroupApplyBack, this, _1, _2));
}

void Client::ConnectionCallBack(const TcpConnectionPtr& conn)
{
    if (conn->connected()) 
    {
        LOG_INFO << "connection sucessful " << conn->peerAddress().toIpPort();
    } 
    else
    {
        LOG_INFO << "Connection disconnected: " << conn->peerAddress().toIpPort();
        client_.getLoop()->quit();
    }
}

void Client::OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time) 
{
    auto msgopt = codec_.tryDecode(buffer);

    if (!msgopt.has_value())
    {
        LOG_INFO << "Recv from " << conn->peerAddress().toIpPort() << " failed";
        return;
    }

    auto [type, js] = msgopt.value();

    LOG_INFO << js.dump();

    dispatcher_.dispatch(type, conn, js, time);
}

void Client::MessageCompleteCallback(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        LOG_INFO << conn->getLoop() << " Loop and send messages to " << conn->peerAddress().toIpPort();
    }
}

void Client::Register(const json& js)
{
    this->send(codec_.encode(js, "Register"));
}

void Client::RegisterBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);
    if (end)
    {
        std::cout << "注册成功" << std::endl;
    }
    else
    {
        std::cout << "注册失败" << std::endl;
    }
    notifyInputReady();
}

void Client::Login(const json& js)
{
    this->send(codec_.encode(js, "Login"));
}

void Client::LoginBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    int id = 0;
    std::string name;
    AssignIfPresent(js, "end", end);
    AssignIfPresent(js, "id", id);
    AssignIfPresent(js, "name", name);
    if (end)
    {
        std::cout << "登陆成功" << std::endl;
        currentState_ = "main_menu";
        userid_ = id;
        name_ = name;
    }
    else
    {
        std::cout << "登陆失败" << std::endl;
    }
    notifyInputReady();
}

void Client::SendMessage(const json& js)
{
    this->send(codec_.encode(js, "SendMessage"));
}

void Client::SendMessageBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);
    if (end)
    {
        // std::cout << "Send successful" << std::endl;
    }
    else
    {
        std::cout << "发送失败" << std::endl;
    }
    notifyInputReady();
}

void Client::RecvMessageBack(const TcpConnectionPtr& conn, const json& js)
{
    std::string sendername;
    std::string content;
    std::string timestamp;

    AssignIfPresent(js, "sendername", sendername);
    AssignIfPresent(js, "content", content);
    AssignIfPresent(js, "timestamp", timestamp);     
    
    std::cout << sendername << ": " << content << std::endl;
}

void Client::GetChatHistory(const json& js)
{
    this->send(codec_.encode(js, "ChatHistory"));
}

void Client::GetChatHistoryBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        bool news = true;
        int senderid;
        int receiverid;
        std::string content;
        std::string status;
        std::string timestamp;

        for (auto it : js["messages"])
        {
            AssignIfPresent(it, "senderid", senderid);
            AssignIfPresent(it, "receiverid", receiverid);
            AssignIfPresent(it, "content", content);
            AssignIfPresent(it, "status", status);
            AssignIfPresent(it, "timestamp", timestamp);

            if (senderid == userid_)
            {
                std::cout << "用户: " << content << "时间: "<< timestamp << std::endl;
            }
            else  
            {
                if (news && status == "Unread")
                {
                    std::cout << "以下为新消息" << std::endl;
                    news = false;
                }
                std::string friendname = friendlist_[senderid].name_;
                std::cout << friendname << ": " << content << "     "<< timestamp << std::endl;
            }
        }
    }
    else  
    {
        std::cout << "获取历史消息失败" << std::endl;
    }
    
    notifyInputReady();
}

void Client::GetGroupHistory(const json& js)
{
    this->send(codec_.encode(js, "GroupHistory"));
}

void Client::GetGroupHistoryBack(const TcpConnectionPtr& conn, const json& js)
{

}

void Client::SendApply(const json& js)
{
    this->send(codec_.encode(js, "SendFriendApply"));
}

void Client::SendFriendApplyBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    
    AssignIfPresent(js, "end", end);

    if (end == true)
    {
        std::cout << "发送好友申请成功" << std::endl;
    }
    else
    {
        std::cout << "发送好友申请失败" << std::endl;
    }

    notifyInputReady();
}

void Client::ListAllApply(const json& js)
{
    this->send(codec_.encode(js, "ListFriendApply"));
}

void Client::ListFriendApplyBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        int fromid;
        std::string fromname;
        std::string status;
        std::unordered_map<int, FriendApply> newfriendapplylist_;
        for (auto it : js["friendapplys"])
        {
            AssignIfPresent(it, "fromid", fromid);
            AssignIfPresent(it, "fromname", fromname);
            AssignIfPresent(it, "status", status);
            newfriendapplylist_.emplace(fromid, FriendApply(fromid, fromname, status));
        }
        friendapplylist_ = std::move(newfriendapplylist_);
    }
    else  
    {
        std::cout << "获取好友申请列表失败" << std::endl;
    }
    
    notifyInputReady();
}
        
void Client::ListAllSendApply(const json& js)
{
    this->send(codec_.encode(js, "ListSendFriendApply"));
}

void Client::ListSendFriendApplyBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        int targetid;
        std::string targetname;
        std::string status;
        std::unordered_map<int, FriendApply> newfriendsendapplylist_;
        for (auto it : js["friendsendapplys"])
        {
            AssignIfPresent(it, "target", targetid);
            AssignIfPresent(it, "targetname", targetname);
            AssignIfPresent(it, "status", status);
            FriendApply fdapply(targetid, targetname, status);
            newfriendsendapplylist_.emplace(targetid, FriendApply(targetid, targetname, status));
        }
        friendsendapplylist_ = std::move(newfriendsendapplylist_);
    }
    else  
    {
        std::cout << "获取发送好友申请列表失败" << std::endl;
    }
    
    notifyInputReady();
}

void Client::ProceFriendApply(const json& js)
{
    this->send(codec_.encode(js, "ProceFriendApply"));
}

void Client::ProceFriendApplyBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;

    AssignIfPresent(js, "end", end);

    if (end == true)
    {
        std::cout << "处理好友申请成功" << std::endl;
    }
    else  
    {
        std::cout << "处理好友申请失败" << std::endl;
    }
    notifyInputReady();
}

void Client::ListFriend(const json& js)
{
    this->send(codec_.encode(js, "ListFriend"));
}

void Client::ListFriendBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        int friendid;
        std::string friendname;
        bool block;
        std::unordered_map<int, Friend> newfriendlist_;

        for (auto it : js["friends"])
        {
            AssignIfPresent(it, "friendid", friendid);
            AssignIfPresent(it, "friendname", friendname);
            AssignIfPresent(it, "block", block);

            Friend fd(friendid, friendname, block);
            newfriendlist_.emplace(friendid, Friend(friendid, friendname, block));
        }
        friendlist_ = std::move(newfriendlist_);
    }
    else  
    {
        std::cout << "获取好友列表失败" << std::endl;
    }
    
    notifyInputReady();
}

void Client::CreateGroup(const json& js)
{
    this->send(codec_.encode(js, "CreateGroup"));
}

void Client::CreateGroupBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        std::cout << "创建群聊成功" << std::endl;
    }
    else  
    {
        std::cout << "创建群聊失败" << std::endl;
    }

    notifyInputReady();
}

void Client::SendGroupApply(const json& js)
{
    this->send(codec_.encode(js, "SendGroupApply"));
}

void Client::SendGroupApplyBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        std::cout << "发送群聊申请成功" << std::endl;
    }
    else  
    {
        std::cout << "发送群聊申请失败" << std::endl;
    }

    notifyInputReady();
}

void Client::ListGroupApply(const json& js)
{
    this->send(codec_.encode(js, "ListGroupApply"));
}

void Client::ListGroupApplyBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        int applyid;
        std::string applyname;
        std::string status;
        for (auto it : js["groupapplys"])
        {
            AssignIfPresent(it, "userid", applyid);
            AssignIfPresent(it, "username", applyname);
            AssignIfPresent(it, "status", status);
            std::cout << "申请人id" << "  申请人名字   " << "  状态" << std::endl;
            std::cout << applyid << "     " << applyname << "     " << status << std::endl;
        }
    }   
    else  
    {
        std::cout << "获取群聊申请列表失败" << std::endl;
    }

    notifyInputReady();
}

void Client::ProceGroupApply(const json& js)
{
    this->send(codec_.encode(js, "ProceGroupApply"));
}

void Client::ProceGroupApplyBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        std::cout << "处理用户申请成功" << std::endl;
    }
    else  
    {
        std::cout << "处理用户申请失败" << std::endl;
    }

    notifyInputReady();
}

void Client::ListSendGroupApply(const json& js)
{
    this->send(codec_.encode(js, "ListSendGroupApply"));
}

void Client::ListSendGroupApplyBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        int groupid;
        std::string groupname;
        std::string status;
        for (auto it : js["groupsendapplys"])
        {
            AssignIfPresent(it, "groupid", groupid);
            AssignIfPresent(it, "groupname", groupname);
            AssignIfPresent(it, "status", status);
            std::cout << "申请群聊id" << "  申请群聊名字   " << "  状态" << std::endl;
            std::cout << groupid << "     " << groupname << "     " << status << std::endl;
        }
    }
    else  
    {
        std::cout << "获取申请的群聊列表失败" << std::endl;
    }

    notifyInputReady();
}

void Client::GroupMember(const json& js)
{
    this->send(codec_.encode(js, "ListGroupMember"));
}

void Client::GroupMemberBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        int userid;
        std::string username;
        std::string role;
        bool mute;
        std::cout << "成员id" << "  成员名字   " << "  身份  " << " 是否禁言 "<< std::endl;
        for (auto it : js["groupmembers"])
        {
            AssignIfPresent(it, "userid", userid);
            AssignIfPresent(it, "username", username);
            AssignIfPresent(it, "role", role);
            AssignIfPresent(it, "mute", mute);
            std::cout << userid << "    " << username << "           " << role << "     " << mute << std::endl;
        }
    }
    else  
    {
        std::cout << "获取群聊成员失败" << std::endl;
    }

    notifyInputReady();
}

void Client::ListGroup(const json& js)
{
    this->send(codec_.encode(js, "ListGroup"));
}

void Client::ListGroupBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        int groupid;
        std::string groupname;
        std::string role;
        std::unordered_map<int, Group> newgrouplist;
        for (auto it : js["groups"])
        {
            AssignIfPresent(it, "groupid", groupid);
            AssignIfPresent(it, "groupname", groupname);
            AssignIfPresent(it, "role", role);
            std::cout << "群聊id" << "  群聊名字   " << "  角色  "<< std::endl;
            std::cout << groupid << "     " << groupname << "     " << role << std::endl;
            newgrouplist.emplace(groupid, Group(groupid, groupname, role));
        }
        grouplist_ = std::move(newgrouplist);
    }
    else  
    {
        std::cout << "获取群聊列表失败" << std::endl;
    }

    notifyInputReady();
}

void Client::QuitGroup(const json& js)
{
    this->send(codec_.encode(js, "QuitGroup"));
}

void Client::QuitGroupBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        std::cout << "退出群聊成功" << std::endl;
    }
    else  
    {
        std::cout << "退出群聊失败" << std::endl;
    }

    notifyInputReady();
}

void Client::ChangeUserRole(const json& js)
{
    this->send(codec_.encode(js, "ChangeUserRole"));
}

void Client::ChangeUserRoleBack(const TcpConnectionPtr& conn, const json& js)
{
    bool end = false;
    AssignIfPresent(js, "end", end);

    if (end)
    {
        std::cout << "改变用户角色成功" << std::endl;
    }
    else  
    {
        std::cout << "改变用户角色失败" << std::endl;
    }

    notifyInputReady();
}

void Client::start()
{
    client_.connect();
}

void Client::stop()
{
    client_.disconnect();
    client_.getLoop()->quit();
}


void Client::send(const std::string& msg)
{
    client_.getLoop()->runInLoop([this, msg]() {
        if (client_.connection() && client_.connection()->connected())
        {
            client_.connection()->send(msg);
        }
    });
}

inline void Client::waitInPutReady()
{
    if (waitingback_)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !waitingback_; });
    }
}

inline void Client::notifyInputReady() 
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        waitingback_ = false;
    }
    cv_.notify_one();
}

void Client::InputLoop()
{
    while (true)
    {
        if (currentState_ == "init_menu")
        {
            int order = 0;
            std::string input;
            std::cout << "1. 登陆" << std::endl;
            std::cout << "2. 注册" << std::endl;
            std::cout << "3. 退出" << std::endl;
            getline(std::cin, input);
            if (!ReadNum(input, order)) continue;

            if (order == 1)
            {
                std::cout << "输入邮箱: ";
                getline(std::cin, email_);
                std::cout << "输入密码: ";
                getline(std::cin, password_);

                json js = {
                    {"email", email_},
                    {"password", password_}
                };

                waitingback_ = true;
                Login(js);

                waitInPutReady();

            }
            else if (order == 2)
            {
                std::cout << "输入邮箱: ";
                getline(std::cin, email_);
                std::cout << "输入用户名: ";
                getline(std::cin, name_);
                std::cout << "输入密码: ";
                getline(std::cin, password_);

                json js = {
                    {"username", name_},
                    {"email", email_},
                    {"password", password_}
                };

                waitingback_ = true;
                Register(js);

                waitInPutReady();
            }
            else if (order == 3)
            {
                std::cout << "正在退出..." << std::endl;
                stop();
                break;
            }
            else  
            {
                std::cout << "输入错误" << std::endl;
            }
        }
        else if (currentState_ == "main_menu")
        {
            int order;
            std::string input;
            std::cout << "1. 添加好友" << std::endl;
            std::cout << "2. 查看好友列表" << std::endl;
            std::cout << "3. 查看好友申请列表" << std::endl;
            std::cout << "4. 处理好友申请" << std::endl;
            std::cout << "5. 查看被申请好友列表" << std::endl;
            std::cout << "6. 私聊" << std::endl;
            std::cout << "7. 群聊" << std::endl;
            std::cout << "8. 退出" << std::endl;
            getline(std::cin, input);
            if (!ReadNum(input, order)) continue;

            if (order == 1)
            {
                std::string email;
                std::cout << "输入要添加的好友email:";
                getline(std::cin, email);
                json js = {
                    {"fromid", userid_},
                    {"targetemail", email}
                };
                waitingback_ = true;
                SendApply(js);

                waitInPutReady();
            }
            else if (order == 2)
            {
                json js = {
                    {"userid", userid_}
                };
                waitingback_ = true;
                ListFriend(js);

                waitInPutReady();
                for (auto it : friendlist_)
                {
                    std::cout << "id: "<< it.first << " name: " << it.second.name_ << std::endl;
                }
            }
            else if (order == 3)
            {
                json js = {
                    {"targetid", userid_}
                };
                waitingback_ = true;
                ListAllApply(js);

                waitInPutReady();
                for (auto it : friendapplylist_)
                {
                    std::cout << "id: " << it.first << " name: " << it.second.name_ << " status: " << it.second.status_ << std::endl;
                }
            }
            else if (order == 4)
            {
                json js = {
                    {"targetid", userid_}
                };
                waitingback_ = true;
                ListAllApply(js);

                waitInPutReady();
                for (auto it : friendsendapplylist_)
                {
                    if (it.second.status_ == "Pending")
                    {
                        std::cout << "id: " << it.first << " name: " << it.second.name_ << std::endl;
                    }
                }
                while (true)
                {
                    int fromid;
                    std::string result;
                    std::string input;
                    std::cout << "输入处理的申请用户id(输入0返回): ";
                    getline(std::cin, input);
                    if (!ReadNum(input, fromid)) continue;

                    if (fromid == 0) break;
                    std::cout << "输入选择(Agree或Reject): ";
                    getline(std::cin, result);
                    if (!(result == "Agree" || result == "Reject"))
                    {
                        std::cout << "输入错误,请重新操作" << std::endl;
                        continue;
                    }

                    json js = {
                        {"fromid", fromid},
                        {"targetid", userid_},
                        {"status", result}
                    };
                    waitingback_ = true;

                    ProceFriendApply(js);
                    waitInPutReady();
                }
                
            }
            else if (order == 5)
            {
                json js = {
                    {"fromid", userid_}
                };
                waitingback_ = true;
                ListAllSendApply(js);

                waitInPutReady();
                for (auto it : friendsendapplylist_)
                {
                    std::cout << "id: " << it.first << " name: " << it.second.name_ << " status: " << it.second.status_ << std::endl;
                }
            }
            else if (order == 6)
            {
                ClearScreen();
                currentState_ = "friendchat_menu";
            }
            else if (order == 7)
            {
                ClearScreen();
                currentState_ = "group_menu";
            }
            else if (order == 8)
            {
                std::cout << "正在退出..." << std::endl;
                stop();
                break;
            }
            else
            {
                std::cout << "错误指令,请重试" << std::endl;
            }
        }
        else if (currentState_ == "friendchat_menu")
        {
            int friendid;
            std::string input;
            std::cout << "输入聊天的好友id(输入0退出): ";
            getline(std::cin, input);
            if (!ReadNum(input, friendid)) continue;
            if (friendid == 0) currentState_ = "main_menu";

            // if (friendlist_.find(friendid) == friendlist_.end())
            // {
            //     std::cout << "未知好友id" << std::endl;
            //     currentState_ = "main_menu";
            //     continue;
            // }

            json js = {
                {"userid", userid_},
                {"friendid", friendid},
            };
            waitingback_ = true;
            GetChatHistory(js);

            waitInPutReady();

            while (true)
            {
                std::string message;
                std::cout << "输入(输入0返回): " ;
                getline(std::cin, message);
                if (message == "0") break;

                json js = {
                    {"senderid", userid_},
                    {"sendername", name_},
                    {"receiverid", friendid},
                    {"content", message},
                    {"type", "Private"},
                    {"status", "Unread"},
                };

                waitingback_ = true;
                SendMessage(js);

                waitInPutReady();
            }

            ClearScreen();
        }
        else if (currentState_ == "group_menu")
        {
            int order = 0;
            std::string input;
            std::cout << "1. 创建群聊" << std::endl;
            std::cout << "2. 加入群聊" << std::endl;
            std::cout << "3. 查看群聊列表" << std::endl;
            std::cout << "4. 查看发送的群聊申请" << std::endl;
            std::cout << "5. 进入群聊" << std::endl;
            std::cout << "6. 返回上一界面" << std::endl;

            getline(std::cin, input);
            if (!ReadNum(input, order)) continue;

            if (order == 1)
            {
                std::string groupname;
                std::cout << "输入创建的群聊名称(输入0返回): ";
                getline(std::cin, groupname);
                if (groupname == "0") continue;

                json js = {
                    {"groupname", groupname},
                    {"ownerid", userid_}
                };

                waitingback_ = true;
                CreateGroup(js);

                waitInPutReady();
            }
            else if (order == 2)
            {
                std::string groupname;
                std::cout << "输入要加入的群聊名称(输入0返回): ";
                getline(std::cin, groupname);
                if (groupname == "0") continue;

                json js = {
                    {"groupname", groupname},
                    {"userid", userid_}
                };

                waitingback_ = true;
                SendGroupApply(js);

                waitInPutReady();
            }
            else if (order == 3)
            {
                json js = {
                    {"userid", userid_}
                };

                waitingback_ = true;
                ListGroup(js);

                waitInPutReady();
            }
            else if (order == 4)
            {
                json js = {
                    {"userid", userid_}
                };

                waitingback_ = true;
                ListSendGroupApply(js);

                waitInPutReady();
            }
            else if (order == 5)
            {
                currentState_ = "groupchat_menu";
                continue;;
            }
            else  
            {
                currentState_ = "main_menu";
                continue;;
            }
        }
        else if (currentState_ == "groupchat_menu")
        {
            std::string input;
            int order;
            int groupid;
            std::cout << "输入要进入的群聊id" << std::endl;
            getline(std::cin, input);
            if (!ReadNum(input, groupid)) continue;
                
            ClearScreen();
            while (true) 
            {
                std::cout << "1. 聊天" << std::endl;
                std::cout << "2. 查看群聊成员" << std::endl;
                std::cout << "3. 查看群聊申请" << std::endl;
                std::cout << "4. 处理群聊申请" << std::endl;
                std::cout << "5. 更改群聊成员权限" << std::endl;
                std::cout << "6. 退出群聊" << std::endl;
                std::cout << "7. 返回上一界面" << std::endl;

                getline(std::cin, input);
                if (!ReadNum(input, order)) continue;

                if (order == 1)
                {
                    while (true)
                    {
                        std::string message;
                        std::cout << "输入(输入0返回): " ;
                        getline(std::cin, message);
                        if (message == "0") break;

                        json js = {
                            {"senderid", userid_},
                            {"sendername", name_},
                            {"receiverid", groupid},
                            {"content", message},
                            {"type", "Group"},
                            {"status", "Unread"},
                        };

                        waitingback_ = true;
                        SendMessage(js);

                        waitInPutReady();
                    }

                    ClearScreen();
                }
                else if (order == 2)
                {
                    json js = {
                        {"groupid", groupid}
                    };

                    waitingback_ = true;
                    GroupMember(js);

                    waitInPutReady();

                }
                else if (order == 3)
                {
                    json js = {
                        {"groupid", groupid}
                    };

                    waitingback_ = true;
                    ListGroupApply(js);

                    waitInPutReady();
                }
                else if (order == 4)
                {
                    int applyid;
                    std::string status;
                    std::cout << "输入要处理的申请用户的id: ";
                    getline(std::cin, input);
                    if (!ReadNum(input, applyid)) continue;

                    std::cout << "输入选择(Agree或Reject): ";
                    getline(std::cin, status);
                    if (!(status == "Agree" || status == "Reject"))
                    {
                        std::cout << "输入错误,请重新操作" << std::endl;
                        continue;
                    }

                    json js = {
                        {"groupid", groupid},
                        {"applyid", applyid},
                        {"status", status}
                    };

                    waitingback_ = true;
                    ProceGroupApply(js);

                    waitInPutReady();
                }
                else if (order == 5)
                {
                    int userid;
                    std::string role;
                    std::cout << "输入要改变的成员id: " << std::endl;
                    getline(std::cin, input);
                    if (!ReadNum(input, userid)) continue;
                    std::cout << "输入改变后的角色(Member或Administrator): " << std::endl;
                    getline(std::cin, role);
                    if (!(role == "Member" || role == "Administrator"))
                    {
                        std::cout << "输入错误" << std::endl;
                        continue;
                    }

                    json js = {
                        {"groupid", groupid},
                        {"userid", userid},
                        {"role", role}
                    };

                    waitingback_ = true;
                    ChangeUserRole(js);

                    waitInPutReady();
                }
                else if (order == 6)
                {
                    json js = {
                        {"groupid", groupid},
                        {"userid", userid_}
                    };

                    waitingback_ = true;
                    QuitGroup(js);

                    waitInPutReady();
                    currentState_ = "group_menu";
                    break;
                }
                else if (order == 7)
                {
                    currentState_ = "group_menu";
                    break;
                }
                else  
                {
                    std::cout << "错误指令,请重试" << std::endl;
                }
            }
        }
        else  
        {
            LOG_ERROR << "!!!!Wrong!!!!";
            return;
        }
    }
}
