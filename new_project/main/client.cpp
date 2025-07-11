#include "../muduo/net/tcp/TcpClient.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/InetAddress.h"
#include "../tool/Codec.h"
#include "../tool/Dispatcher.h"
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <unordered_map>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

Timestamp timestamp;
struct Friend {
    Friend(int id, std::string name, bool block)
        : id_(id), name_(name), block_(block) {}
    int id_;
    std::string name_;
    bool block_;
};

struct FriendApply {
    FriendApply(int id, std::string name, std::string status)
        : id_(id), name_(name), status_(status) {}
    int id_;
    std::string name_;
    std::string status_;
};


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

class Client  
{
public:
    Client(EventLoop& loop, InetAddress addr, std::string name)
        : client_(&loop, addr, name)
    {
        client_.setConnectionCallback(std::bind(&Client::ConnectionCallBack, this, _1));
        client_.setMessageCallback(std::bind(&Client::OnMessage, this, _1, _2, _3));
        client_.setWriteCompleteCallback(std::bind(&Client::MessageCompleteCallback, this, _1));

        dispatcher_.registerHander("RegisterBack", std::bind(&Client::RegisterBack, this, _1, _2, _3));
        dispatcher_.registerHander("LoginBack", std::bind(&Client::LoginBack, this, _1, _2, _3));
        dispatcher_.registerHander("SendMessageBack", std::bind(&Client::SendMessageBack, this, _1, _2, _3));
        dispatcher_.registerHander("RecvMessage", std::bind(&Client::RecvMessageBack, this, _1, _2, _3));
        dispatcher_.registerHander("SendApplyBack", std::bind(&Client::SendApplyBack, this, _1, _2, _3));
        dispatcher_.registerHander("AllApplyBack", std::bind(&Client::ListAllApplyBack, this, _1, _2, _3));
        dispatcher_.registerHander("AllSendApplyBack", std::bind(&Client::ListAllSendApplyBack, this, _1, _2, _3));
        dispatcher_.registerHander("ProceApplyBack", std::bind(&Client::ProceApplyBack, this, _1, _2, _3));
        dispatcher_.registerHander("ListFriendBack", std::bind(&Client::ListFriendBack, this, _1, _2, _3));
    }

    void ConnectionCallBack(const TcpConnectionPtr& conn)
    {
        if (conn->connected()) 
        {
            LOG_INFO << "connection sucessful " << conn->peerAddress().toIpPort();
        } 
        else
        {
            LOG_INFO << "Connection disconnected: " << conn->peerAddress().toIpPort();
        }
    }

    void OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time) 
    {
        auto msgopt = codec_.tryDecode(buffer);

        if (!msgopt.has_value())
        {
            LOG_INFO << "Recv from " << conn->peerAddress().toIpPort() << " failed";
            return;
        }

        auto [type, js] = msgopt.value();

        dispatcher_.dispatch(type, conn, js, time);
    }

    void MessageCompleteCallback(const TcpConnectionPtr& conn)
    {
        if (conn->connected())
        {
            LOG_INFO << conn->getLoop() << " Loop and send messages to " << conn->peerAddress().toIpPort();
        }
    }

    void Register(const json& js)
    {
        this->send(codec_.encode(js, "Register"));
    }

    void RegisterBack(const TcpConnectionPtr& conn, const json& js, Timestamp time)
    {
        bool end = false;
        AssignIfPresent(js, "end", end);
        if (end)
        {
            std::cout << "Register successful" << std::endl;
        }
        else
        {
            std::cout << "Register failed" << std::endl;
        }
        notifyInputReady();
    }

    void Login(const json& js)
    {
        this->send(codec_.encode(js, "Login"));
    }

    void LoginBack(const TcpConnectionPtr& conn, const json& js, Timestamp time)
    {
        bool end = false;
        int id = 0;
        std::string name;
        AssignIfPresent(js, "end", end);
        AssignIfPresent(js, "id", id);
        AssignIfPresent(js, "name", name);
        if (end)
        {
            std::cout << "Login successful" << std::endl;
            currentState_ = "main_menu";
            userid_ = id;
            name_ = name;
        }
        else
        {
            std::cout << "Login failed" << std::endl;
        }
        notifyInputReady();
    }

    void SendMessage(const json& js)
    {
        this->send(codec_.encode(js, "SendMessage"));
    }

    void SendMessageBack(const TcpConnectionPtr& conn, const json& js, Timestamp time)
    {
        bool end = false;
        AssignIfPresent(js, "end", end);
        if (end)
        {
            std::cout << "Send successful" << std::endl;
        }
        else
        {
            std::cout << "Send failed" << std::endl;
        }
        notifyInputReady();
    }

    void RecvMessageBack(const TcpConnectionPtr& conn, const json& js, Timestamp time)
    {
        int sendid = 0;
        int receiverid = 0;
        std::string content;
        std::string type;
        std::string status;
        std::string timestamp;

        AssignIfPresent(js, "senderid", sendid);
        AssignIfPresent(js, "receiverid", receiverid);
        AssignIfPresent(js, "content", content);
        AssignIfPresent(js, "type", type);
        AssignIfPresent(js, "status", status);
        AssignIfPresent(js, "timestamp", timestamp);     
        
        std::cout << "Recv: " << content << std::endl;
    }

    void SendApply(const json& js)
    {
        this->send(codec_.encode(js, "SendApply"));
    }

    void SendApplyBack(const TcpConnectionPtr& conn, const json& js, Timestamp time)
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

    void ListAllApply(const json& js)
    {
        this->send(codec_.encode(js, "AllApply"));
    }

    void ListAllApplyBack(const TcpConnectionPtr& conn, const json& js, Timestamp time)
    {
        int fromid;
        std::string fromname;
        std::string status;
        std::unordered_map<int, FriendApply> newfriendapplylist_;

        for (auto it : js)
        {
            AssignIfPresent(it, "fromid", fromid);
            AssignIfPresent(it, "fromname", fromname);
            AssignIfPresent(it, "status", status);

            newfriendapplylist_.emplace(fromid, FriendApply(fromid, fromname, status));
        }

        friendapplylist_ = std::move(newfriendapplylist_);
        notifyInputReady();
    }

    void ListAllSendApply(const json& js)
    {
        this->send(codec_.encode(js, "AllSendApply"));
    }

    void ListAllSendApplyBack(const TcpConnectionPtr& conn, const json& js, Timestamp time)
    {
        int targetid;
        std::string targetname;
        std::string status;
        std::unordered_map<int, FriendApply> newfriendsendapplylist_;

        for (auto it : js)
        {
            AssignIfPresent(it, "target", targetid);
            AssignIfPresent(it, "targetname", targetname);
            AssignIfPresent(it, "status", status);

            FriendApply fdapply(targetid, targetname, status);
            newfriendsendapplylist_.emplace(targetid, FriendApply(targetid, targetname, status));
        }

        friendsendapplylist_ = std::move(newfriendsendapplylist_);
        notifyInputReady();
    }

    void ProceApply(const json& js)
    {
        this->send(codec_.encode(js, "ProceApply"));
    }

    void ProceApplyBack(const TcpConnectionPtr& conn, const json& js, Timestamp time)
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

    void ListFriend(const json& js)
    {
        this->send(codec_.encode(js, "ListFriend"));
    }

    void ListFriendBack(const TcpConnectionPtr& conn, const json& js, Timestamp time)
    {
        int friendid;
        std::string friendname;
        bool block;
        std::unordered_map<int, Friend> newfriendlist_;

        for (auto it : js)
        {
            AssignIfPresent(it, "friendid", friendid);
            AssignIfPresent(it, "friendname", friendname);
            AssignIfPresent(it, "block", block);

            Friend fd(friendid, friendname, block);
            newfriendlist_.emplace(friendid, Friend(friendid, friendname, block));
        }
        friendlist_ = std::move(newfriendlist_);
        notifyInputReady();
    }

    void start()
    {
        client_.connect();
    }

    void send(const std::string& msg)
    {
        client_.getLoop()->runInLoop([this, msg]() {
            if (client_.connection() && client_.connection()->connected())
            {
                client_.connection()->send(msg);
            }
        });
    }

    inline void waitInPutReady()
    {
        if (waitingback_)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !waitingback_; });
        }
    }

    inline void notifyInputReady() 
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            waitingback_ = false;
        }
        cv_.notify_one();
    }

    void InputLoop()
    {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
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
                std::cout << "4. 查看待处理好友申请列表" << std::endl;
                std::cout << "5. 查看被申请好友列表" << std::endl;
                std::cout << "6. 私聊" << std::endl;
                std::cout << "7. 退出" << std::endl;
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

                        ProceApply(js);
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
                    currentState_ = "chat_menu";
                    //进入聊天界面，获取历史聊天记录
                }
                else if (order == 7)
                {
                    break;
                }
                else
                {
                    std::cout << "错误指令,请重试" << std::endl;
                }
            }
            else if (currentState_ == "chat_menu")
            {
                int friendid;
                std::string input;
                std::cout << "输入聊天的好友id(输入0退出): ";
                getline(std::cin, input);
                if (!ReadNum(input, friendid)) continue;

                if (friendlist_.find(friendid) == friendlist_.end())
                {
                    std::cout << "未知好友id" << std::endl;
                    currentState_ = "main_menu";
                    continue;
                }

                while (true)
                {
                    std::string message;
                    std::cout << "输入(输入q返回): " ;
                    getline(std::cin, message);
                    if (message == "q") break;

                    json js {
                        {"senderid", userid_},
                        {"receiverid", friendid},
                        {"content", message},
                        {"type", "Private"},
                        {"status", "Unread"},
                       {"timestamp", Gettime()}
                    };

                    waitingback_ = true;
                    SendMessage(js);

                    waitInPutReady();
                }

                ClearScreen();
            }
            else  
            {
                LOG_ERROR << "!!!!Wrong!!!!";
                return;
            }
        }
    }
private:
    std::atomic<bool> waitingback_ = false;
    std::string currentState_ = "init_menu";
    std::mutex mutex_;
    std::condition_variable cv_;

    TcpClient client_;
    Codec codec_;
    Dispatcher dispatcher_;

    int userid_ = 0;
    std::string email_;
    std::string name_;
    std::string password_;
    //id,name
    std::unordered_map<int, Friend> friendlist_;
    std::unordered_map<int, FriendApply> friendapplylist_;
    std::unordered_map<int, FriendApply> friendsendapplylist_;
};

int main ()
{
    EventLoop loop;
    InetAddress addr(8080, "127.0.0.1");

    Client client(loop, addr, "ChatClient");

    client.start();

    std::thread th([&client]() {
        client.InputLoop();
    });

    loop.loop();
}