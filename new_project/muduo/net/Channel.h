#ifndef CHANNEL_H
#define CHANNEL_H

#include "../base/Timestamp.h"
#include "../base/noncopyable.h"
#include "../logging/Logging.h"

#include <functional>
#include <memory>

class EventLoop;
class Timestamp;

// channel是连接文件描述符与事件循环之间的桥梁
// 事件循环检测有事件发生，通知channel
// channel在监听的文件描述符上调用注册的回调函数
// 通常一个文件描述符对应一个channel

class Channel {
  public:
    using EventCallback = std::function<void()>; // 普通的回调函数
    using ReadEventCallback =
        std::function<void(Timestamp)>; // 读事件回调函数（有时间戳）

    Channel(EventLoop *loop, int fd);
    ~Channel();

    void handleEvent(
        Timestamp receiveTime); // 收到Poller的通知，由EventLoop调用回调函数

    void setReadCallback(ReadEventCallback cb) {
        readCallback_ = std::move(cb);
    } // 设置读回调函数
    void setWriteCallback(EventCallback cb) {
        writeCallback_ = std::move(cb);
    } // 设置写回调函数
    void setCloseCallback(EventCallback cb) {
        closeCallback_ = std::move(cb);
    } // 设置关闭文件描述符回调函数
    void setErrorCallback(EventCallback cb) {
        errorCallback_ = std::move(cb);
    } // 设置错误回调函数

    // 绑定一个对象的shared_ptr，用于检查对象是否还活着
    void tie(const std::shared_ptr<void> &);

    int fd() const { return fd_; }         // 返回channel对应的文件描述符
    int events() const { return events_; } // 返回监听的事件
    void set_revents(int revt) {
        revents_ = revt;
    } // Poller通知我实际发生的事件

    // 设置相对应的事件，调用update通知Poller更新
    void enableReading() {
        events_ |= kReadEvent;
        update();
    }
    void disableReading() {
        events_ &= ~kReadEvent;
        update();
    }
    void enableWriting() {
        events_ |= kWriteEvent;
        update();
    }
    void disableWriting() {
        events_ &= ~kWriteEvent;
        update();
    }
    void disableAll() {
        events_ = kNoneEvent;
        update();
    }

    // 检查当前是否注册了某种事件
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isWriting() const { return events_ & kWriteEvent; }
    bool isReading() const { return events_ & kReadEvent; }

    int index() {
        return index_;
    } // Poller使用，记录该Channel是否被加入到Poller中，记录该Channel的状态
    void set_index(int idx) { index_ = idx; } // 记录该Channel已加入Poller

    EventLoop *ownerLoop() { return loop_; } // 返回所属的Eventloop
    void remove();                           // 从Poller中移除该Channel

  private:
    void update(); // Poller更新
    void
    handleEventWithGuard(Timestamp receiveTime); // 真正回调前检查tie_是否有效

    static const int kNoneEvent;  // 不关心任何事件
    static const int kReadEvent;  // 关心读事件，包括新连接
    static const int kWriteEvent; // 关心写事件

    EventLoop *loop_; // Channel对应的Eventloop
    const int fd_;    // Poller监听的文件描述符
    int events_;      // 注册fd感兴趣的事件
    int revents_;     // Poller通知实际发生的事件
    int index_;       // Poller记录的当前Channel的信息

    std::weak_ptr<void> tie_; // 弱指针指向TcpConnection
    bool tied_;               // 是否调用过tie_

    ReadEventCallback readCallback_; // 读事件回调函数
    EventCallback writeCallback_;    // 写事件回调函数
    EventCallback closeCallback_;    // 关闭文件描述符回调函数
    EventCallback errorCallback_;    // 发生错误回调函数
};
#endif