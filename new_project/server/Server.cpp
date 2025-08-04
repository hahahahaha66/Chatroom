#include "Server.h"
#include "../muduo/logging/Logging.h"
#include "Service.h"
#include <functional>

Server::Server(EventLoop* loop,const InetAddress& listenAddr, const InetAddress& fileAddr, std::string name)
    : server_(loop, listenAddr, name), 
      fileserver_(loop, fileAddr, "fileserver"),
      service_(loop)
{
    MysqlConnectionPool::Instance().Init("localhost", 3306, "hahaha", "123456", "chatroom", 48);

    server_.setThreadNum(16);

    server_.setConnectionCallback(std::bind(&Server::OnConnection, this, _1));
    server_.setMessageCallback(std::bind(&Server::OnMessage, this, _1, _2, _3));
    server_.setWriteCompleteCallback(std::bind(&Server::MessageCompleteCallback, this, _1));
    server_.setThreadInitCallback(std::bind(&Server::ThreadInitCallback, this, _1));

    RegisterHandlerSafe(dispatcher_, "Register", service_, &Service::UserRegister);
    RegisterHandlerSafe(dispatcher_, "Login", service_, &Service::UserLogin);
    RegisterHandlerSafe(dispatcher_, "DeleteAccount", service_, &Service::DeleteAccount);
    RegisterHandlerSafe(dispatcher_, "Flush", service_, &Service::Flush);
    RegisterHandlerSafe(dispatcher_, "SendMessage", service_, &Service::MessageSend);
    RegisterHandlerSafe(dispatcher_, "ChatHistory", service_, &Service::GetChatHistory);
    RegisterHandlerSafe(dispatcher_, "GroupHistory", service_, &Service::GetGroupHistory);
    RegisterHandlerSafe(dispatcher_, "ChanceInterFace", service_, &Service::ChanceInterFace);

    RegisterHandlerSafe(dispatcher_, "SendFriendApply", service_, &Service::SendFriendApply);
    RegisterHandlerSafe(dispatcher_, "ListFriendApply", service_, &Service::ListFriendApply);
    RegisterHandlerSafe(dispatcher_, "ListSendFriendApply", service_, &Service::ListSendFriendApply);
    RegisterHandlerSafe(dispatcher_, "ProceFriendApply", service_, &Service::ProceFriendApply);
    RegisterHandlerSafe(dispatcher_, "ListFriend", service_, &Service::ListFriend);
    RegisterHandlerSafe(dispatcher_, "BlockFriend", service_, &Service::BlockFriend);
    RegisterHandlerSafe(dispatcher_, "DeleteFriend", service_, &Service::BlockFriend);

    RegisterHandlerSafe(dispatcher_, "CreateGroup", service_, &Service::CreateGroup);
    RegisterHandlerSafe(dispatcher_, "SendGroupApply", service_, &Service::SendGroupApply);
    RegisterHandlerSafe(dispatcher_, "ListGroupApply", service_, &Service::ListGroupApply);
    RegisterHandlerSafe(dispatcher_, "ListSendGroupApply", service_, &Service::ListSendGroupApply);
    RegisterHandlerSafe(dispatcher_, "ListGroupMember", service_, &Service::ListGroupMember);
    RegisterHandlerSafe(dispatcher_, "ListGroup", service_, &Service::ListGroup);
    RegisterHandlerSafe(dispatcher_, "QuitGroup", service_, &Service::QuitGroup);
    RegisterHandlerSafe(dispatcher_, "ChangeUserRole", service_, &Service::ChangeUserRole);
    RegisterHandlerSafe(dispatcher_, "ProceGroupApply", service_, &Service::ProceGroupApply);
    RegisterHandlerSafe(dispatcher_, "DeleteGroup", service_, &Service::DeleteGroup);
    RegisterHandlerSafe(dispatcher_, "BlockGroupUser", service_, &Service::BlockGroupUser);
    RegisterHandlerSafe(dispatcher_, "RemoveGroupUser", service_, &Service::RemoveGroupUser);

    RegisterHandlerSafe(dispatcher_, "AddFileToMessage", service_, &Service::AddFileToMessage);
    RegisterHandlerSafe(dispatcher_, "ListFriendFile", service_, &Service::ListFriendFile);
    RegisterHandlerSafe(dispatcher_, "ListGroupFile", service_, &Service::ListGroupFile);
}

void Server::start()
{
    server_.start();
    fileserver_.start();
}

void Server::ThreadInitCallback(EventLoop* loop)
{
    
}

void Server::MessageCompleteCallback(const TcpConnectionPtr& conn)
{
    if (conn->connected())
    {
        LOG_DEBUG << conn->getLoop() << " Loop and send messages to " << conn->peerAddress().toIpPort();
    }
}

void Server::OnConnection(const TcpConnectionPtr& conn) 
{
    if (conn->connected()) 
    {
        LOG_INFO << "New connection from " << conn->peerAddress().toIpPort();
    } 
    else
    {
        LOG_INFO << "Connection disconnected: " << conn->peerAddress().toIpPort();
        service_.RemoveUserConnect(conn);
    }
}

void Server::OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time) 
{
    LOG_DEBUG << "tryDecode: buffer size = " << buffer->readableBytes();

    auto msgopt = codec_.tryDecode(buffer);

    if (!msgopt.has_value())
    {
        LOG_INFO << "Recv from " << conn->peerAddress().toIpPort() << " failed";
        return ;
    }

    const auto& parsed = *msgopt;
    const auto& type = std::get<0>(parsed);
    const auto& js = std::get<1>(parsed);

    LOG_INFO << "Start Processing " << type;

    if (type == "GetFileServerPort")
    {
        uint16_t fileport;
        std::string addr = fileserver_.GetServerPort();
        size_t pos = addr.find(':');
        if (pos != std::string::npos) 
        {
            std::string port_str = addr.substr(pos + 1);
            fileport = stoi(port_str);
        }
        else 
        {
            LOG_ERROR << "Invalid address format";
        }
        std::cout << fileport <<  std::endl;
        json j = {
            {"port", fileport}
        };
        for (auto& user : service_.GetOnlineUserList())
        {
            if (user.second.GetConn() == conn)
                user.second.SetIsTransferFiles(true);
        }

        conn->send(codec_.encode(j, "GetFileServerPortBack"));
        return;
    }
    if (type != "Flush")
    {
        for (auto& user : service_.GetOnlineUserList())
        {
            if (user.second.GetConn() == conn)
            {
                user.second.SetIsTransferFiles(false);
                user.second.SetTimeOut(0);
            }
        }
    }

    dispatcher_.dispatch(type, conn, js);
}