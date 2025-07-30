#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "../../base/noncopyable.h"
#include "InetAddress.h"

#include <functional>
#include <memory>

class Channel;
class EventLoop;

//用来抽象一条客户端的连接
class Connector : noncopyable, public std::enable_shared_from_this<Connector>
{
public:
    using NewConnectionCallback = std::function<void(int sockfd)>;

    Connector(EventLoop* loop, const InetAddress& serverAddr);
    ~Connector();

    //设置连接成功后的回调
    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { newConnectionCallback_ = cb; }

    //启动连接
    void start();
    //重新启动连接
    void restart();
    //停止连接
    void stop();

    //获取服务器地址
    const InetAddress& serverAddress() const { return serverAddr_; }

    //获取套接字的本地地址
    static struct sockaddr_in getLocalAddr(int sockfd);

    //获取套接字的对端地址
    static struct sockaddr_in getPeerAddr(int sockfd);

    //用于检查是否发生了自连接
    static bool isSelfConnect(int sockfd);

    //获取套接字的错误状态
    static int getSocketError(int sockfd);

private:
    //表示连接状态
    enum States
    {
        kDisconnected,  //未连接
        kConnecting,  //正在连接
        kConnected  //已连接
    };
    static const int kMaxRetryDelayMs = 30 * 1000;  //最大重试延迟
    static const int kInitRetryDelayMs = 500;  //初始重试延迟

    //设置连接状态
    void setStates(States s) { state_ = s; }
    //在事件循环中启动连接
    void startInLoop();
    //在事件循环中停止连接
    void stopInLoop();
    //连接请求
    void connect();
    //处理连接
    void connecting(int sockfd);
    //处理写操作
    void handleWrite();
    //处理错误操作
    void handleError();
    //重新尝试连接
    void retry(int sockfd);
    //移除并重置channel
    int removeAndResetChannel();
    //重置channel
    void resetChannel();

    EventLoop* loop_;  //客户端所在的事件循环
    InetAddress serverAddr_;  //服务器地址
    bool connect_;  //是否正在连接
    States state_;  //连接状态
    std::unique_ptr<Channel> channel_;  //指向客户端的channel
    NewConnectionCallback newConnectionCallback_;  //连接成功后的回调
    int retryDelayMs_;  //重试的延迟时间

};


#endif