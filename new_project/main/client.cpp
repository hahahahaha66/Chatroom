#include "../muduo/net/tcp/TcpClient.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/InetAddress.h"
#include "../tool/Codec.h"
#include "../tool/Dispatcher.h"
#include <atomic>
#include <condition_variable>
#include <iostream>
#include <thread>
#include <unistd.h>

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

Timestamp timestamp;

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
        AssignIfPresent(js, "end", end);
        AssignIfPresent(js, "id", id);
        if (end)
        {
            std::cout << "Login successful" << std::endl;
            currentState_ = "chat_menu";
            userid = id;
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
            if (waitingback_)
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this] { return !waitingback_; });
            }

            if (currentState_ == "main_menu")
            {
                std::string username, password, email;
                std::this_thread::sleep_for(std::chrono::seconds(2));
                json js;
                std::cout << "email: ";
                std::cin >> email;
                std::cout << "Password: ";
                std::cin >> password;

                js["email"] = email;
                js["password"] = password;

                waitingback_ = true;

                this->Login(js);
            }
            else if (currentState_ == "chat_menu")
            {
                int receiverid;
                std::cout << "receiverid: ";
                std::cin >> receiverid;
                
                std::string message;
                std::cout << "Input Message: " ;
                std::cin >> message;

                //获取时间戳
                auto now = std::chrono::system_clock::now();
                auto time_t = std::chrono::system_clock::to_time_t(now);
                std::string timestamp_str = std::ctime(&time_t);
                // 移除换行符
                if (!timestamp_str.empty() && timestamp_str.back() == '\n') {
                    timestamp_str.pop_back();
                }

                json js {
                    {"senderid", userid},
                    {"receiverid", receiverid},
                    {"content", message},
                    {"type", "Private"},
                    {"status", "Unread"},
                   {"timestamp", timestamp_str}
                };

                waitingback_ = true;
                SendMessage(js);
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
    std::string currentState_ = "main_menu";
    std::mutex mutex_;
    std::condition_variable cv_;

    TcpClient client_;
    Codec codec_;
    Dispatcher dispatcher_;

    int userid = 0;
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