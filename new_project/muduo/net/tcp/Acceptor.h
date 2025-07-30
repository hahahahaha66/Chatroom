#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include "../../base/noncopyable.h"
#include "../Channel.h"
#include "InetAddress.h"
#include "Socket.h"

class EventLoop;
class InetAddress;

class Acceptor 
{
public:
    using NewConnectionCallback = std::function<void(int sockfd, const InetAddress&)>;

    Acceptor(EventLoop* loop, const InetAddress& ListenAddr, bool reuseport);
    ~Acceptor();

    //设置新连接的回调函数
    void setNewConnectionCallback(const NewConnectionCallback& cb)
    {
        NewConnectionCallback_ = cb;
    }

    //查看当前是否监听
    bool listenning() const { return listenning_; }
    //开始监听
    void listen();

private:

    //处理新连接的读事件
    void handleRead();

    EventLoop* loop_;  //主Eventloop
    Socket acceptSocket_;
    Channel acceptChannel_;
    NewConnectionCallback NewConnectionCallback_;  //回调函数
    bool listenning_;
};

#endif