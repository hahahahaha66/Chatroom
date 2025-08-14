#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "../../base/Timestamp.h"
#include "../../base/noncopyable.h"
#include "../Buffer.h"
#include "Callback.h"
#include "Socket.h"

#include <asm-generic/socket.h>
#include <atomic> //原子变量
#include <cerrno>
#include <cstddef>
#include <functional>
#include <memory> //智能指针
#include <string>
#include <sys/socket.h> //socket基本函数
#include <sys/types.h>
#include <unistd.h>

class Channel;
class EventLoop;
class Socket;

// 该类用来抽象一条Tcp连接
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<
                          TcpConnection> // 允许再类内可以拿到自己的shared_ptr
{
  public:
    TcpConnection(EventLoop *loop, const std::string &nameArg, int sockfd,
                  const InetAddress &localAddr, const InetAddress &peerAddr);

    ~TcpConnection();

    // 查询是哪条EventLoop
    EventLoop *getLoop() const { return loop_; }

    // const修饰一下函数返回值，无法通过引用修改对应的值

    // 获取该Tcp连接的名字，方便日志打印
    const std::string &name() const { return name_; }
    // 获取本地地址
    const InetAddress &localAddress() const { return localAddr_; }
    // 获取对端地址
    const InetAddress &peerAddress() const { return peerAddr_; }

    int fd() const { return socket_->fd(); }

    // 设置连接状态
    bool connected() const { return state_ == kConnected; }

    // 发送数据
    void send(const std::string &buf);
    void send(Buffer *buf);

    // 半关闭连接，只关闭写端
    void shutdown();

    // 强制关闭
    void forceClose();

    void forceCloseInLoop();

    // 设置自定义的回调函数，控制事件发生后的行为
    void setConnectionCallback(const ConnectionCallback &cb) {
        connectionCallback_ = cb;
    }
    void setMessageCallback(const MessageCallback &cb) {
        messageCallback_ = cb;
    }
    void setWriteCompleteCallback(const WriteCompleteCallback &cb) {
        writecompletecallback_ = cb;
    }
    void setCloseCallback(const CloseCallback &cb) { closecallback_ = cb; }
    void setHighWaterMarkCallback(const HighWaterMarkCallback &cb,
                                  size_t highWaterMark) {
        highwatermarkcallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    // 获取水位大小
    size_t getHighWaterMark() { return highWaterMark_; }

    // 获取收或发的缓冲区
    const Buffer &getInputBuffer() { return inputBuffer_; }
    const Buffer &getOutputBuffer() { return outputBuffer_; }

    void connectEstablished(); // 连接建立
    void connectDestroyed();   // 连接销毁

  private:
    enum StateE {
        kDisconnected,  // 已经断开连接
        kConnecting,    // 正在连接
        kConnected,     // 已连接
        kDisconnecting, // 正在断开连接
    };
    void setState(StateE state) { state_ = state; }

    // 注册不同事件的回调函数
    // 可读事件
    void handleRead(Timestamp receiveTime);
    // 可写事件
    void handleWrite();
    // 对端关闭
    void handleClose();
    // 发生错误
    void handleError();

    // 真正实现发送和关闭
    void sendInLoop(const void *message, size_t len);
    void sendInLoop(const std::string &message);
    void shutdownInLoop();

    EventLoop *loop_;        // 属于哪个EventLoop
    const std::string name_; // 该Tcp连接的名字
    std::atomic_int state_;  // 当前连接状态
    bool reading_;

    std::unique_ptr<Socket> socket_;   // 已连接的socket文件描述符
    std::unique_ptr<Channel> channel_; // 连接的channel

    const InetAddress localAddr_; // 本地地址
    const InetAddress peerAddr_;  // 对端地址

    ConnectionCallback connectionCallback_;       // 新连接的回调
    MessageCallback messageCallback_;             // 有读写消息的回调
    WriteCompleteCallback writecompletecallback_; // 数据发送完成以后的回调
    CloseCallback closecallback_;                 // 客户端关闭的回调
    HighWaterMarkCallback highwatermarkcallback_; // 超出水位的回调

    size_t highWaterMark_; // 水位线，到达这个数量就会触发一个回调函数

    Buffer inputBuffer_;
    Buffer outputBuffer_;
};

#endif