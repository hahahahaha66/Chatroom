#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "../EventLoop.h"
#include "../EventLoopThreadPool.h"
#include "Acceptor.h"
#include "InetAddress.h"
#include "../../base/noncopyable.h"
#include "Callback.h"
#include "TcpConnection.h"

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>

class TcpServer : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    enum Option
    {
        kNoReusePort,  //端口不复用
        kReusePort,  //端口复用
    };

    TcpServer(EventLoop* loop, const InetAddress& ListenAddr, const std::string& nameArg, Option option = kNoReusePort);
    ~TcpServer();

    //设置不同的回调函数
    //线程初始化
    void setThreadInitCallback(const ThreadInitCallback& cb) { threadInitCallback_ = cb; }
    //有新连接到来
    void setConnectionCallback(const ConnectionCallback& cb) { connectionCallback_ = cb; }
    //消息到来时
    void setMessageCallback(const MessageCallback& cb) { messageCallback_ = cb; }
    //发送消息完成
    void setWriteCompleteCallback(const WriteCompleteCallback& cb) { writeCompleteCallback_ = cb; }
    //调节发送水位
    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb, size_t highWaterMark)
    {
        highwatermarkcallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    //设置线程数量
    void setThreadNum(int numThreads);

    //启动Tcpserver
    void start();

    //获取服务器的EventLoop
    EventLoop* getLoop() const { return loop_; }

    //获取服务器名字
    const std::string name() { return name_; }

    //服务器监听的端口号
    const std::string inPort() { return inPort_; }

private:
    //处理新连接
    void newConnection(int sockfd, const InetAddress& peerAddr);
    //移除连接，用户调用
    void removeConnection(const TcpConnectionPtr& conn);
    //真正从EventLoop中移除连接
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    //连接和Tcp的映射
    using ConnectionMap = std::unordered_map<std::string, TcpConnectionPtr>;

    EventLoop* loop_;  //服务器所属的主循环
    const std::string inPort_;  //监听的端口号
    const std::string name_;  //服务器名字
    std::unique_ptr<Acceptor> acceptor_;  //负责监听新连接

    std::shared_ptr<EventLoopThreadPool> threadPool_;  //管理线程池

    ConnectionCallback connectionCallback_;  //新连接回调
    MessageCallback messageCallback_;  //有读消息时的回调
    WriteCompleteCallback writeCompleteCallback_;  //发送消息完成时的回调
    HighWaterMarkCallback highwatermarkcallback_;  //发送水位调节
    size_t highWaterMark_;

    ThreadInitCallback threadInitCallback_;  //线程初始化的回调
    std::atomic_int started_;  //是否启动服务器

    int nextConnId_;
    ConnectionMap connections_;  //保存所有活跃的连接
};

#endif