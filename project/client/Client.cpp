#include "Client.h"
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

Client::~Client()
{
    client_.stop();
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

    if (seq == 0)
    {
        //处理消息的刷新包
        clientservice_.RefreshMessage(js, time);
    }
    else if (seq < 0)
    {
        //处理其他逻辑的刷新包
        switch (seq) 
        {
            case -1 :
                clientservice_.RefreshFriendDelete(js, time);
            break;
            case -2 :
                clientservice_.RefreshFriendBlock(js, time);
            break;
            case -3 :
                clientservice_.RefreshFriendAddApply(js, time);
            break;
            case -4 :
                clientservice_.RefreshGroupAddApply(js, time);
            break;
            case -5 :
                clientservice_.RefreshGroupCreate(js, time);
            break;
            case -6 :
                clientservice_.RefreshFriendApplyProcess(js, time);
            break;
            case -7 :
                clientservice_.RefreshGroupApplyProcess(js, time);
            break;
            case -8 :
                clientservice_.RefreshGroupQuitUser(js, time);
            break;
            case -9 :
                clientservice_.RefreshGroupRemoveUser(js, time);
            break;
            case -10 :
                clientservice_.RefreshGroupAddAdministrator(js, time);
            break;
            case -11 :
                clientservice_.Back_RemoveAdministrator(js, time);
            break;
            default:
                std::cout << "未识别的消息包格式" << std::endl;
        }
    }
    else  
    {
        processpend_.executeRequest(seq, js, time);
    }
}

bool Client::ReadingIntegers(std::string input, int &result)
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

void Client::InitialInterface()
{
    while (true)
    {
        std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;
        std::cout << "1. 登陆" << std::endl;
        std::cout << "2. 注册" << std::endl;

        std::string order_;
        std::getline(std::cin, order_);
        
        if (order_ == "1")
        {

            while (true)
            {
                std::string username;
                std::string password;
                std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;

                std::cout << "请输入登陆的账户名字 : ";
                std::getline(std::cin, username);
                if (username == "q") break;
                if (username.size() > 20)
                {
                    std::cout << "用户名太长,请重新操作" << std::endl;
                    continue;
                }

                std::cout << "请输入账户密码 : ";
                std::getline(std::cin, password);
                if (password == "q") break;
                if (password.size() > 20)
                {
                    std::cout << "密码输入太长,请重新操作" << std::endl;
                    continue;
                }

                //~
                //登陆成功成功调用一下函数，否则循环
                if (1)
                {
                    //~
                    //初始化个人信息
                    MainInterface();
                    break;
                }
                
            }
        }
        else if (order_ == "2")
        {
            while (true) 
            {
                std::string username;
                std::string password;
                std::string password_again;
                std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;

                std::cout << "请输入注册的账户名字(10位以内) : ";
                std::getline(std::cin, username);
                if (username == "q") break;
                if (username.size() > 20)
                {
                    std::cout << "用户名太长,请重新操作" << std::endl;
                    continue;
                }
                
                std::cout << "请输入账户密码 : ";
                std::getline(std::cin, password);
                if (password == "q") break;
                if (password.size() > 20)
                {
                    std::cout << "密码输入太长,请重新操作" << std::endl;
                    continue;
                }

                std::cout << "请再次输入账户密码 : ";
                std::getline(std::cin, password);
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
        else if (order_ == "q")
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

        std::string order_;
        std::getline(std::cin, order_);

        if (order_ == "1")
        {
            FriendInterface();
        }
        else if (order_ == "2")
        {
            GroupInterface();
        }
        else if (order_ == "3")
        {
            //~
            //打印个人数据
            //可以本地打印
        }
        else if (order_ == "q")
        {
            break;
        }
        else  
        {
            std::cout << "错误的命名输入,请重新输入" << std::endl;
        }
    }
}

void Client::FriendInterface()
{
    while (true)
    {
        std::string order_;
        std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;
        std::cout << "1. 选择好友聊天" << std::endl;
        std::cout << "2. 添加好友" << std::endl;
        std::cout << "3. 删除好友" << std::endl;

        std::getline(std::cin, order_);

        if (order_ == "1")
        {
            //~
            //打印朋友列表
            //本地打印
            int friendid = 0;
            std::cout << "输入要聊天的朋友id : ";
            std::string input;
            std::getline(std::cin, input);
            if (ReadingIntegers(input, friendid) && friendid != 0)
            {
                ClearScreen();
                //~
                //获取历史记录
                PrivateChatInterface(friendid);
            }
        }
        else if (order_ == "2")
        {
            int new_friendid = 0;
            std::cout << "输入新的朋友的id : ";
            std::string input;
            std::getline(std::cin, input);
            if (ReadingIntegers(input, new_friendid) && new_friendid != 0)
            {
                //~
                //发送好友申请
            }
        }
        else if (order_ == "3")
        {
            int old_friendid = 0;
            //打印朋友列表
            std::cout << "输入要删除的朋友 : ";
            std::string input;
            std::getline(std::cin, input);
            if (ReadingIntegers(input, old_friendid) && old_friendid != 0)
            {
                //检查该id是否在好友列表中
                //~
                //删除好友
            }
        }
        else if (order_ == "q")  
        {
            break;
        }
        else  
        {
            std::cout << "输入错误,请重新输入" << std::endl;
        }
    }
}

void Client::GroupInterface()
{
    while (true)
    {
        std::string order_;
        std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;
        std::cout << "1. 选择群聊聊天" << std::endl;
        std::cout << "2. 加入新群聊" << std::endl;
        std::cout << "3. 退出群聊" << std::endl;
        std::cout << "4. 创建群聊" << std::endl;

        std::getline(std::cin, order_);

        if (order_ == "1")
        {
            //~
            //打印群聊列表
            //本地打印
            int groupid = 0;
            std::cout << "输入要进入的群聊id : ";
            std::string input;
            std::getline(std::cin, input);
            if (ReadingIntegers(input, groupid) && groupid != 0)
            {
                //确认是否是群聊id
                ClearScreen();
                //~
                //获取历史记录
                GroupChatInterface(groupid);
            }
        }
        else if (order_ == "2")
        {
            int new_groupid = 0;
            std::cout << "输入要加入的群聊的id : ";
            std::string input;
            std::getline(std::cin, input);
            std::cout << input;
            if (ReadingIntegers(input, new_groupid) && new_groupid != 0)
            {
                //~
                //发送群聊申请
            }
        }
        else if (order_ == "3")
        {
            int old_groupid = 0;
            //打印群聊列表
            //本地打印
            std::cout << "输入要退出的群聊id : ";
            std::string input;
            std::getline(std::cin, input);
            if (ReadingIntegers(input, old_groupid) && old_groupid != 0)
            {
                //检查是否是已加入的群聊
                //~
                //执行退出群聊操作
            }
        }
        else if (order_ == "4")
        {
            std::string new_groupname;
            std::vector<int> new_groupuserid;

            std::cout << "输入群聊名字 : ";
            std::getline(std::cin, new_groupname);
            
            std::cout << "现在初始化群聊成员" << std::endl;
            //打印好友列表
            //本地打印
            std::cout << "输入好友id,完成输入-1";
            int tempid = 0;

            while (true)
            {
                if (tempid == -1)
                {
                    std::cout << "当前初始化成员列表 : ";
                    for (int& i : new_groupuserid)
                    {
                        std::cout << i << " ";
                    }
                    std::cout << std::endl;
                    break;
                }

                std::string input;
                std::getline(std::cin, input);
                if (ReadingIntegers(input, tempid) && tempid != 0)
                {
                    //检查输入的id是否是朋友，若果不是报错
                    //如果已经输入过报错，可以改用set
                    new_groupuserid.push_back(tempid);
                }

                std::cout << "当前初始化成员列表 : ";
                for (int& i : new_groupuserid)
                {
                    std::cout << i << " ";
                }
                std::cout << std::endl;
            }

            //~
            //创建群聊
        }
        else if (order_ == "q")  
        {
            break;
        }
        else  
        {
            std::cout << "输入错误,请重新输入" << std::endl;
        }
    }
}

void Client::PrivateChatInterface(int friendid)
{
    while (true)
    {
        std::string order_;
        std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;
        std::cout << "1. 发送消息" << std::endl;
        std::cout << "2. 屏蔽好友" << std::endl;
        std::cout << "3. 删除好友" << std::endl;

        std::getline(std::cin, order_);
        
        if (order_ == "1")
        {
            ClearScreen();
            //检查是否屏蔽，如果屏蔽则打印并显示已屏蔽，无法发送消息
            std::cout << "q返回上一界面" << std::endl;
            while (true)
            {
                std::string message;
                std::cout << "我 : ";
                std::getline(std::cin, message);
                if (message == "q")
                {
                    break;
                }
                //~
                //将消息发送给朋友
            }
        }
        else if (order_ == "2")
        {
            //~
            //这里有两种情况
            //若未屏蔽，则设为屏蔽
            //若已屏蔽，则设为未屏蔽
            //将最终结果打印出来
        }
        else if (order_ == "3")
        {
            std::string temp_order;
            std::cout << "1. 确认删除好友" << std::endl;
            std::cout << "2. 取消删除好友" << std::endl;

            std::getline(std::cin, temp_order);

            if (temp_order == "1")
            {
                //~
                //删除好友，并返回上一界面
                break;
            }
        }
        else if (order_ == "q")
        {
            break;
        }
        else  
        {
            std::cout << "命令输入错误,请重新输入" << std::endl;
            return;
        }
    }
}

void Client::GroupChatInterface(int groupid)
{
    while (true)
    {
        std::string order_;

        if (1)  //普通用户
        {
            std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;
            std::cout << "1. 发送消息" << std::endl;
            std::cout << "2. 查看群聊成员" << std::endl;
            std::cout << "3. 退出群聊" << std::endl;
        }
        else if (1)  //管理员
        {
            std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;
            std::cout << "1. 发送消息" << std::endl;
            std::cout << "2. 查看群聊成员" << std::endl;
            std::cout << "3. 退出群聊" << std::endl;
            std::cout << "4. 查看申请列表" << std::endl;
            std::cout << "5. 移除成员" << std::endl;
        
        }
        else  //群主
        {
            std::cout << "-----------输入q退出或返回上一界面----------------" << std::endl;
            std::cout << "1. 发送消息" << std::endl;
            std::cout << "2. 查看群聊成员" << std::endl;
            std::cout << "3. 退出群聊" << std::endl;
            std::cout << "4. 查看申请列表" << std::endl;
            std::cout << "5. 移除成员" << std::endl;
            std::cout << "6. 设置管理员" << std::endl;
            std::cout << "7. 取消管理员" << std::endl;
            std::cout << "8. 删除群聊" << std::endl;
        }

        std::getline(std::cin, order_);

        if (order_ == "1")
        {
            ClearScreen();
            //检查是否屏蔽，如果屏蔽则打印并显示已屏蔽，无法发送消息
            std::cout << "q返回上一界面" << std::endl;
            while (true)
            {
                std::string message;
                std::cout << "我 : ";
                std::getline(std::cin, message);
                if (message == "q")
                {
                    break;
                }
                //~
                //将消息发送到群聊
            }
        }
        else if (order_ == "2")
        {
            //~
            //打印群聊成员
            //本地打印
        }
        else if (order_ == "3")
        {
            std::string sure;
            std::cout << "1. 确定退出群聊" << std::endl;
            std::cout << "2. 取消退出群聊" << std::endl;

            std::getline(std::cin, sure);

            if (sure == "1")
            {
                //~
                //退出群聊
                break;
            }
            else if (sure == "1" || sure == "q")
            {
                continue;
            }
            else  
            {
                std::cout << "输入错误,自动返回上一界面" << std::endl;
            }
        }
        else if (order_ == "4")
        {
            //~
            //打印申请列表
            //本地打印
            while (true)
            {
                std::string temp_order;
                std::cout << "1. 输入批准的成员id" << std::endl;
                std::cout << "2. 返回上一界面" << std::endl;

                std::getline(std::cin, temp_order);

                if (temp_order == "1")
                {
                    int applyid = 0;
                    std::string input;
                    std::cout << "请输入 : ";
                    std::getline(std::cin, input);
                    if (ReadingIntegers(input, applyid) && applyid != 0)
                    {
                    //~
                    //将该申请者加入群聊，并删除申请
                        std::cout << "处理成功" << std::endl;
                    }
                }
                else if (temp_order == "2" || temp_order == "q")
                {
                    break;
                }
                else  
                {
                    std::cout << "输入错误,自动返回上一界面" << std::endl;
                    break;
                }
            }
        }
        else if (order_ == "5")
        {
            std::string temp_order;
            std::cout << "1. 输入移除的成员id" << std::endl;
            std::cout << "2. 返回上一界面" << std::endl;

            std::getline(std::cin, temp_order);

            if (temp_order == "1")
            {
                //~
                //打印成员列表
                int removeid = 0;
                std::cout << "请输入 : " << std::endl;
                std::string input;
                std::getline(std::cin, input);
                if (ReadingIntegers(input, removeid) && removeid != 0)
                {
                    //检查是否是成员id
                    //是则移除
                    //不是则报错
                }
            }
            else if (temp_order == "2" || temp_order == "q")
            {
                break;
            }
            else  
            {
                std::cout << "输入错误,自动返回上一界面" << std::endl;
                break;
            }
        }
        else if (order_ == "6")
        {
            while (true)
            {
                std::string temp_order;
                std::cout << "1. 输入新的管理员id" << std::endl;
                std::cout << "2. 返回上一界面" << std::endl;

                std::getline(std::cin, temp_order);

                if (temp_order == "1")
                {
                    int new_administratorid = 0;
                    //~
                    //打印群聊成员列表
                    std::cout << "请输入 : ";
                    std::string input;
                    std::getline(std::cin, input);
                    if (ReadingIntegers(input, new_administratorid) && new_administratorid != 0)
                    {
                        //判断是否是群聊成员
                        //~
                        //执行设置管理员
                    }
                }
                else if (temp_order == "2" || temp_order == "q")
                {
                    break;
                }
                else  
                {
                    std::cout << "输入错误,自动返回上一界面" << std::endl;
                    break;
                }
            }
        }
        else if (order_ == "7")
        {
            while (true)
            {
                std::string temp_order;
                std::cout << "1. 输入取消的管理员id" << std::endl;
                std::cout << "2. 返回上一界面" << std::endl;

                std::getline(std::cin, temp_order);

                if (temp_order == "1")
                {
                    int old_administratorid = 0;
                    //~
                    //打印管理员员列表
                    std::cout << "请输入 : ";
                    std::string input;
                    std::getline(std::cin, input);
                    if (ReadingIntegers(input, old_administratorid) && old_administratorid != 0)
                    {
                        //判断是否是管理员员
                        //~
                        //执行取消管理员
                    }
                }
                else if (temp_order == "2" || temp_order == "q")
                {
                    break;
                }
                else  
                {
                    std::cout << "输入错误,自动返回上一界面" << std::endl;
                    break;
                }
            }
        }                        
        else if (order_ == "8")
        {
            std::string sure;
            std::cout << "1. 确定删除群聊" << std::endl;
            std::cout << "2. 取消删除群聊" << std::endl;

            std::getline(std::cin, sure);

            if (sure == "1")
            {
                //~
                //删除群聊
                break;
            }
            else if (sure == "1" || sure == "q")
            {
                continue;
            }
            else  
            {
                std::cout << "输入错误,自动返回上一界面" << std::endl;
            }
        }        
        else if (order_ == "q")
        {
            break;
        }
        else  
        {
            std::cout << "命令错误,请重新输入" << std::endl;
        }
    }
}