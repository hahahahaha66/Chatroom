#ifndef TCPCLIENT_C
#define TCPCLIENT_C

#include "Callback.h"
#include "InetAddress.h"
#include "TcpConnection.h"
#include "Connector.h"
#include <memory>
#include <mutex>

class Connector;

class TcpClient : noncopyable
{
public:
    using ConnectorPtr = std::shared_ptr<Connector>;

    TcpClient(EventLoop* loop, const InetAddress& serverAddr, const std::string& nameArg);
    ~TcpClient();

    //发起连接
    void connect();
    //断开连接
    void disconnect();
    //停止客户端工作
    void stop();

    bool connected()
    {
        return connect_;
    }

    //获取当前连接
    TcpConnectionPtr connection() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return connection_;
    }

    EventLoop* getLoop() const { return loop_; }
    bool retry() const { return retry_; }
    void enableRetry() { retry_ = true; }

    const std::string& name() const { return name_; }

    void setConnectionCallback(ConnectionCallback cb) { connectionCallback_ = std::move(cb); }
    void setMessageCallback(MessageCallback cb) { messageCallback_ = std::move(cb); }
    void setWriteCompleteCallback(WriteCompleteCallback cb) { writeCompleteCallback_ = std::move(cb); }

private:
    //连接建立时调用，创建新的TcpConnection添加到TcpClient
    void newConnection(int sockfd);

    //连接断开时调用，移除TcpConnection
    void removeConnection(const TcpConnectionPtr& conn);

    EventLoop* loop_;  //TcpClient所属的事件循环
    ConnectorPtr connector_;  //Connector指针，基类负责底层功能的实现
    const std::string name_;  //客户端名字
    ConnectionCallback connectionCallback_;  //连接成功后回调
    MessageCallback messageCallback_;   //接收到消息的回调函数
    WriteCompleteCallback writeCompleteCallback_;  //写操作完成的回调函数
    bool retry_;  //是否开启连接重联
    bool connect_;  //是否连接，表示连接状态

    int nextConnId_;  //下一个连接的id（一个客户端可能与多个服务器连接，所以会有下一个id）
    mutable std::mutex mutex_;  //互斥锁
    TcpConnectionPtr connection_ ;  //TcpConnection连接

};

#endif