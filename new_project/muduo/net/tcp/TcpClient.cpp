#include "TcpClient.h"
#include "../../logging/Logging.h"
#include "../EventLoop.h"
#include "Callback.h"
#include "Connector.h"
#include "InetAddress.h"
#include "Socket.h"

#include <functional>
#include <memory>
#include <mutex>
#include <stdio.h>

namespace detail {
using ConnectorPtr = std::shared_ptr<Connector>;

void removeConnection(EventLoop *loop, const TcpConnectionPtr &conn) {
    loop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}

void removeConnector(const ConnectorPtr &connector) {
    // connector->
}
} // namespace detail

TcpClient::TcpClient(EventLoop *loop, const InetAddress &serverAddr,
                     const std::string &nameArg)
    : loop_(loop), connector_(new Connector(loop, serverAddr)), name_(nameArg),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback), retry_(false), connect_(true),
      nextConnId_(1) {
    connector_->setNewConnectionCallback(
        std::bind(&TcpClient::newConnection, this, std::placeholders::_1));
    LOG_DEBUG << "TcpClinet::TcpLient[" << name_ << "] - connector "
              << static_cast<const void *>(connector_.get());
}

TcpClient::~TcpClient() {
    LOG_DEBUG << "TcpClient::~TcpClient[" << name_ << "] - connector "
              << static_cast<const void *>(connector_.get());
    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        unique = connection_.unique(); // 判断是否是唯一引用者
        conn = connection_;            // 增加引用计数
    }

    if (conn) // 还存在连接对象
    {
        CloseCallback cb =
            std::bind(&detail::removeConnection, loop_, std::placeholders::_1);
        loop_->runInLoop(std::bind(&TcpConnection::setCloseCallback, conn, cb));
        if (unique) {
            conn->forceClose();
        }
    } else // 只用当前一个连接对象
    {
        connector_->stop();
        loop_->runAfter(1, std::bind(&detail::removeConnector, connector_));
    }
}

void TcpClient::connect() {
    LOG_DEBUG << "TcpClient::connect[" << name_ << "] - connecting to"
              << connector_->serverAddress().toIpPort();
    connect_ = true;
    connector_->start();
}

void TcpClient::disconnect() {
    connect_ = false;

    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connect_) {
            connection_->shutdown();
        }
    }
}

void TcpClient::stop() {
    connect_ = false;
    connector_->stop();
}

void TcpClient::newConnection(int sockfd) {
    InetAddress peerAddr(Connector::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof(buf), ":%s%d", peerAddr.toIpPort().c_str(),
             nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    InetAddress localAddr(Connector::getLocalAddr(sockfd));

    // 建立新的TcpConnection
    TcpConnectionPtr conn(
        new TcpConnection(loop_, connName, sockfd, localAddr, peerAddr));

    // 设置回调函数
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(
        std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));

    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr &conn) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
    if (retry_ && connect_) {
        LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
                 << connector_->serverAddress().toIpPort();
        connector_->restart();
    }
}
