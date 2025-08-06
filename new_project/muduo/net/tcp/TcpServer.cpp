#include "TcpServer.h"
#include "../../logging//Logging.h"
#include "Acceptor.h"
#include "Callback.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include <cstdio>
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

static EventLoop *CheckLoopNotNull(EventLoop *loop) {
    if (loop == nullptr) {
        LOG_FATAL << "mainloop is null";
    }
    return loop;
}

TcpServer::TcpServer(EventLoop *loop, const InetAddress &listenAddr,
                     const std::string &nameArg, Option option)
    : loop_(CheckLoopNotNull(loop)), inPort_(listenAddr.toIpPort()),
      name_(nameArg),
      acceptor_(new Acceptor(loop, listenAddr, option == kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_)), connectionCallback_(),
      messageCallback_(), writeCompleteCallback_(), threadInitCallback_(),
      started_(0), nextConnId_(1) {
    // 管理新连接
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection,
                                                  this, std::placeholders::_1,
                                                  std::placeholders::_2));
}

TcpServer::~TcpServer() {
    for (auto &item : connections_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TcpConnection::connectDestroyed, conn));
    }
}

void TcpServer::setThreadNum(int numsThreads) {
    threadPool_->setThreadNum(numsThreads);
}

void TcpServer::start() {
    if (started_++ == 0) {
        threadPool_->start(threadInitCallback_);
        loop_->runInLoop(std::bind(&Acceptor::listen, acceptor_.get()));
    }
}

void TcpServer::newConnection(int sockfd, const InetAddress &peerAddr) {
    // 获得下一个连接的EventLoop
    EventLoop *ioLoop = threadPool_->getNextLoop();
    char buf[64] = {0}; // 名字

    snprintf(buf, sizeof(buf), "-%s#%d", inPort_.c_str(), nextConnId_);
    ++nextConnId_;

    std::string connName = name_ + buf;

    LOG_INFO << "Tcpserver::newConnection [" << name_.c_str()
             << "] - new connection [" << connName.c_str() << "] from "
             << peerAddr.toIpPort().c_str();

    sockaddr_in local;
    ::memset(&local, 0, sizeof(local));
    socklen_t addrlen = sizeof(local);
    if (::getsockname(sockfd, (sockaddr *)&local, &addrlen) < 0) {
        LOG_ERROR << "sockets::getLocalAddr() failed";
    }

    InetAddress localAddr(local);
    // 建立一个新的TcpConnection
    TcpConnectionPtr conn(
        new TcpConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    // 存入
    connections_[connName] = conn;
    // 设置回调函数
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setHighWaterMarkCallback(highwatermarkcallback_, highWaterMark_);

    conn->setCloseCallback(
        std::bind(&TcpServer::removeConnectionInLoop, this, conn));

    ioLoop->runInLoop(std::bind(&TcpConnection::connectEstablished, conn));
}

void TcpServer::removeConnection(const TcpConnectionPtr &conn) {
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));
}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr &conn) {
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_.c_str()
             << "] - connection " << conn->name().c_str();
    connections_.erase(conn->name());
    EventLoop *ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TcpConnection::connectDestroyed, conn));
}
