#include "Client.h"
#include "Clientservice.h"
#include <functional>
#include <ostream>
#include <string>

Client::Client(EventLoop* loop, InetAddress& addr, std::string name)
    : loop_(loop), client_(loop, addr, name)
{
    client_.connect();

    client_.setConnectionCallback(std::bind(&Client::ConnectionCallback, this, _1));
    client_.setMessageCallback(std::bind(&Client::MessageCallback, this, _1, _2, _3));
    client_.setWriteCompleteCallback(std::bind(&Client::WriteCompleteCallback, this, _1));

    InitialInterface();
}

void Client::ConnectionCallback(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        std::cout << "连接成功" << std::endl;
    }
    else  
    {
        std::cout << "连接断开" <<std::endl;
    }

}

void Client::WriteCompleteCallback(const TcpConnectionPtr& conn)
{

}

void Client::MessageCallback(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
    auto msgopt = codec_.tryDecode(buf);
    if (!msgopt.has_value())
    {
        std::cout << "消息发送失败" << std::endl;
        return ;
    }
    auto [type, seq, js] = msgopt.value();
    if (type == 0)
    {
        std::cout << "命令执行失败" << std::endl;
    }
    processpend_.executeRequest(seq, js, time);
}

void Client::InitialInterface()
{
    while (true)
    {
        std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;
        std::cout << "1. 登陆" << std::endl;
        std::cout << "2. 注册" << std::endl;
        std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;

        char order_ = 0;
        std::cin >> order_;

        if (order_ == '1')
        {

            while (true)
            {
                std::string username;
                std::string password;
                std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;

                std::cout << "请输入登陆的账户名字 : ";
                std::cin >> username;
                if (username == "q") break;

                std::cout << "请输入账户密码 : ";
                std::cin >> password;
                if (password == "q") break;

                //~
                //登陆成功成功调用一下函数，否则循环
                MainInterface();
            }
        }
        else if (order_ == '2')
        {
            while (true) 
            {
                std::string username;
                std::string password;
                std::string password_again;
                std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;

                std::cout << "请输入注册的账户名字 : ";
                std::cin >> username;
                if (username == "q") break;
                
                std::cout << "请输入账户密码 : ";
                std::cin >> password;
                if (password == "q") break;

                std::cout << "请再次输入账户密码 : ";
                std::cin >> password_again;
                if (password == "q") break;

                if (password != password_again)
                {
                    std::cout << "两次输入密码不同,请重新操作" << std::endl;
                    continue;
                }

                //~
                //注册成功后自动返回上级界面
            }
        }
        else if (order_ == 'q')
        {
            break;
        }
        else 
        {
            std::cout << "输入错误，请重新输入" << std::endl;
        }
    }
}

void Client::MainInterface()
{
    while (true)
    {
        std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;
        std::cout << "1. 私聊" << std::endl;
        std::cout << "2. 群聊" << std::endl;
        std::cout << "3. 个人信息" << std::endl;
        char order_;
        std::cin >> order_;
        if (order_ == '1')
        {
            //~
            //打印朋友列表
            int friendid;
            std::cout << "输入要聊天的朋友id : ";
            std::cin >> friendid;
            //确认是否是朋友id
            ClearScreen();
            //~
            //获取历史记录
            PrivateChatInterface(friendid);
        }
        else if (order_ == '2')
        {
            //~
            //打印群聊列表
            int groupid;
            std::cout << "输入要进入的群聊id : ";
            std::cin >> groupid;
            //确认是否是群聊id
            ClearScreen();
            //~
            //获取历史记录
            GroupChatInterface(groupid);
        }
        else if (order_ == '3')
        {
            //~
            //打印个人数据
        }
        else if (order_ == 'q')
        {
            break;
        }
        else  
        {
            std::cout << "错误的命名输入,请重新输入" << std::endl;
        }
    }
}

void Client::PrivateChatInterface(int friendid)
{
    while (true)
    {
        char order_;
        std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;
        std::cout << "1. 发送消息" << std::endl;
        std::cout << "2. 屏蔽好友" << std::endl;
        std::cout << "3. 删除好友" << std::endl;
        std::cin >> order_;
        
        if (order_ == '1')
        {
            
        }
        else if (order_ == '2')
        {

        }
        else if (order_ == '3')
        {

        }
        else if (order_ == 'q')
        {
            break;
        }
        else  
        {
            std::cout << "命令输入错误,请重新输入" << std::endl;
        }
    }
}
