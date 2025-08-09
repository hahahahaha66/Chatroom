#include "Server.h"
#include "../muduo/logging/Logging.h"
#include "Service.h"
#include <functional>

Server::Server(EventLoop *loop, const InetAddress &listenAddr,
               const InetAddress &fileAddr, std::string name)
    : server_(loop, listenAddr, name),
      fileserver_(loop, fileAddr, "fileserver"), service_(loop) {
    MysqlConnectionPool::Instance().Init("localhost", 3306, "hahaha", "123456",
                                         "chatroom", 48);

    server_.setThreadNum(16);

    server_.setConnectionCallback(std::bind(&Server::OnConnection, this, _1));
    server_.setMessageCallback(std::bind(&Server::OnMessage, this, _1, _2, _3));
    server_.setWriteCompleteCallback(
        std::bind(&Server::MessageCompleteCallback, this, _1));
    server_.setThreadInitCallback(
        std::bind(&Server::ThreadInitCallback, this, _1));

    RegisterHandlerSafe(dispatcher_, "Register", service_,
                        &Service::UserRegister);
    RegisterHandlerSafe(dispatcher_, "Login", service_, &Service::UserLogin);
    RegisterHandlerSafe(dispatcher_, "DeleteAccount", service_,
                        &Service::DeleteAccount);
    RegisterHandlerSafe(dispatcher_, "Flush", service_, &Service::Flush);
    RegisterHandlerSafe(dispatcher_, "SendMessage", service_,
                        &Service::MessageSend);
    RegisterHandlerSafe(dispatcher_, "ChatHistory", service_,
                        &Service::GetChatHistory);
    RegisterHandlerSafe(dispatcher_, "GroupHistory", service_,
                        &Service::GetGroupHistory);
    RegisterHandlerSafe(dispatcher_, "ChanceInterFace", service_,
                        &Service::ChanceInterFace);
    RegisterHandlerSafe(dispatcher_, "SendVerifyCode", service_,
                        &Service::SendVerifyCode);

    RegisterHandlerSafe(dispatcher_, "SendFriendApply", service_,
                        &Service::SendFriendApply);
    RegisterHandlerSafe(dispatcher_, "ListFriendApply", service_,
                        &Service::ListFriendApply);
    RegisterHandlerSafe(dispatcher_, "ListSendFriendApply", service_,
                        &Service::ListSendFriendApply);
    RegisterHandlerSafe(dispatcher_, "ProceFriendApply", service_,
                        &Service::ProceFriendApply);
    RegisterHandlerSafe(dispatcher_, "ListFriend", service_,
                        &Service::ListFriend);
    RegisterHandlerSafe(dispatcher_, "BlockFriend", service_,
                        &Service::BlockFriend);
    RegisterHandlerSafe(dispatcher_, "DeleteFriend", service_,
                        &Service::DeleteFriend);
    RegisterHandlerSafe(dispatcher_, "CheckFriendBlock", service_,
                        &Service::CheckFriendBlock);

    RegisterHandlerSafe(dispatcher_, "CreateGroup", service_,
                        &Service::CreateGroup);
    RegisterHandlerSafe(dispatcher_, "SendGroupApply", service_,
                        &Service::SendGroupApply);
    RegisterHandlerSafe(dispatcher_, "ListGroupApply", service_,
                        &Service::ListGroupApply);
    RegisterHandlerSafe(dispatcher_, "ListSendGroupApply", service_,
                        &Service::ListSendGroupApply);
    RegisterHandlerSafe(dispatcher_, "ListGroupMember", service_,
                        &Service::ListGroupMember);
    RegisterHandlerSafe(dispatcher_, "ListGroup", service_,
                        &Service::ListGroup);
    RegisterHandlerSafe(dispatcher_, "QuitGroup", service_,
                        &Service::QuitGroup);
    RegisterHandlerSafe(dispatcher_, "ChangeUserRole", service_,
                        &Service::ChangeUserRole);
    RegisterHandlerSafe(dispatcher_, "ProceGroupApply", service_,
                        &Service::ProceGroupApply);
    RegisterHandlerSafe(dispatcher_, "DeleteGroup", service_,
                        &Service::DeleteGroup);
    RegisterHandlerSafe(dispatcher_, "BlockGroupUser", service_,
                        &Service::BlockGroupUser);
    RegisterHandlerSafe(dispatcher_, "RemoveGroupUser", service_,
                        &Service::RemoveGroupUser);
    RegisterHandlerSafe(dispatcher_, "AddFriendToGroup", service_,
                        &Service::AddFriendToGroup);

    RegisterHandlerSafe(dispatcher_, "AddFileToMessage", service_,
                        &Service::AddFileToMessage);
    RegisterHandlerSafe(dispatcher_, "ListFriendFile", service_,
                        &Service::ListFriendFile);
    RegisterHandlerSafe(dispatcher_, "ListGroupFile", service_,
                        &Service::ListGroupFile);

    RegisterHandlerSafe(dispatcher_, "ChancePassword", service_,
                        &Service::ChancePassword);
    RegisterHandlerSafe(dispatcher_, "ChanceUserName", service_,
                        &Service::ChanceUsername);
}

void Server::start() {
    server_.start();
    fileserver_.start();
}

void Server::ThreadInitCallback(EventLoop *loop) {}

void Server::MessageCompleteCallback(const TcpConnectionPtr &conn) {
    if (service_.GetSendQueue().find(conn) != service_.GetSendQueue().end()) {
        auto &ctx = service_.GetSendQueue()[conn];
        if (ctx->sending) {
            EventLoop *loop = conn->getLoop();
            loop->runInLoop([this, conn, &ctx]() {
                ctx->pendingmessages.pop();
                if (!ctx->pendingmessages.empty()) {
                    std::string front = ctx->pendingmessages.front();
                    conn->send(front);
                } else {
                    ctx->sending = false;
                }
            });
        }
    }
}

void Server::OnConnection(const TcpConnectionPtr &conn) {
    if (conn->connected()) {
        LOG_INFO << "New connection from " << conn->peerAddress().toIpPort();
    } else {
        LOG_INFO << "Connection disconnected: "
                 << conn->peerAddress().toIpPort();
        service_.RemoveUserConnect(conn);
    }
}

void Server::OnMessage(const TcpConnectionPtr &conn, Buffer *buffer,
                       Timestamp time) {
    while (true) {
        LOG_DEBUG << "tryDecode: buffer size = " << buffer->readableBytes();

        auto msgopt = codec_.tryDecode(buffer);

        if (!msgopt.has_value()) {
            break;
        }

        const auto &parsed = *msgopt;
        const auto &type = std::get<0>(parsed);
        const auto &js = std::get<1>(parsed);

        LOG_INFO << "Start Processing " << type;

        if (type == "GetFileServerPort") {
            uint16_t fileport;
            std::string addr = fileserver_.GetServerPort();
            size_t pos = addr.find(':');
            if (pos != std::string::npos) {
                std::string port_str = addr.substr(pos + 1);
                fileport = stoi(port_str);
            } else {
                LOG_ERROR << "Invalid address format";
            }
            std::cout << fileport << std::endl;
            json j = {{"port", fileport}};
            for (auto &it : service_.GetOnlineUserList()) {
                auto &user = it.second;
                if (user.GetConn() == conn)
                    user.SetIsTransferFiles(true);
            }

            conn->send(codec_.encode(j, "GetFileServerPortBack"));
            return;
        }
        if (type != "Flush") {
            for (auto &it : service_.GetOnlineUserList()) {
                auto &user = it.second;
                if (user.GetConn() == conn) {
                    user.SetIsTransferFiles(false);
                    user.SetTimeOut(0);
                }
            }
        }
        dispatcher_.dispatch(type, conn, js);
    }
}