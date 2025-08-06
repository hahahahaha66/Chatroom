#include "TcpConnection.h"
#include "../../logging/Logging.h"
#include "../Channel.h"
#include "../EventLoop.h"
#include "Callback.h"
#include "InetAddress.h"
#include "Socket.h"

static EventLoop *CheckLoopNotNull(EventLoop *loop) {
    if (loop == nullptr) {
        LOG_FATAL << "mainLoop is null!";
    }
    return loop;
}

TcpConnection::TcpConnection(EventLoop *loop, const std::string &nameArg,
                             int sockfd, const InetAddress &localAddr,
                             const InetAddress &peerAddr)
    : loop_(CheckLoopNotNull(loop)), name_(nameArg), state_(kConnecting),
      reading_(true), socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)), localAddr_(localAddr),
      peerAddr_(peerAddr), highWaterMark_(64 * 1024 * 1024) {
    channel_->setReadCallback(
        std::bind(&TcpConnection::handleRead, this,
                  std::placeholders::_1)); // 使用占位符，表示还有一个参数
    channel_->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TcpConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TcpConnection::handleError, this));

    LOG_INFO << "TcpConnection::ctor[" << name_.data()
             << "] at fd = " << sockfd;
    socket_->setKeepAlive(true);
}

TcpConnection::~TcpConnection() {
    LOG_INFO << "TcpConnection::dtor[" << name_.data() << "] at fd "
             << channel_->fd() << " states= " << static_cast<int>(state_);
}

void TcpConnection::send(const std::string &buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf.data(), buf.size());
        } else {
            // 这里注释的版本有乱码的bug,原因在于异步执行的情况下传递的buf.c_str可能已经被销毁
            // 导致真正实现的时候指针已经指向无效或错误的内存

            // 由于有多个重载版本，需要用函数指针来指明具体是哪个版本
            //  void(TcpConnection::*fp)(const void* data, size_t len) =
            //  &TcpConnection::sendInLoop; loop_->runInLoop(std::bind(fp, this,
            //  buf.c_str(), buf.size()));
            std::string message(buf); // 拷贝一份，延长生命周期
            loop_->runInLoop([this, message]() {
                sendInLoop(message.data(), message.size());
            });
        }
    }
}

void TcpConnection::send(Buffer *buf) {
    if (state_ == kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        } else {
            // 这里也是，由于生命周期的管理不当，导致真正实现的时候指针已经指向无效或错误的内存

            // 理由同上
            //  void (TcpConnection::*fp)(const std::string& massage) =
            //  &TcpConnection::sendInLoop; loop_->runInLoop(std::bind(fp, this,
            //  buf->retrieveAllAsString()));
            std::string message = buf->retrieveAllAsString(); // 拷贝
            loop_->runInLoop([this, message]() { sendInLoop(message); });
        }
    }
}

void TcpConnection::sendInLoop(const std::string &message) {
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void *data, size_t len) {
    ssize_t nwrote = 0;     // 已写入字数
    size_t remaining = len; // 剩余字数
    bool faultError = false;

    // 已断开连接，无法发送数据
    if (state_ == kDisconnected) {
        LOG_ERROR << "disconnected, give up writing";
        return;
    }

    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = ::write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            // 如果数据写完
            if (remaining == 0 && writecompletecallback_) {
                loop_->queueInLoop(
                    std::bind(writecompletecallback_, shared_from_this()));
            }
        } else // 报错
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }

    // 如果数据未写完并且无错
    if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ &&
            highwatermarkcallback_) {
            // 将未写完的事件包装成任务在下一次处理时继续写入
            loop_->queueInLoop(std::bind(highwatermarkcallback_,
                                         shared_from_this(),
                                         oldLen + remaining));
            if (!channel_->isWriting()) {
                channel_->enableWriting();
            }
        }
    }
}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::shutdownInLoop() {
    if (!channel_->isWriting()) {
        socket_->shutdownWrite();
    }
}

void TcpConnection::forceClose() {
    if (state_ == kConnected || state_ == kDisconnecting) {
        setState(kDisconnecting);
        loop_->queueInLoop(
            std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop() {
    if (state_ == kConnected || state_ == kDisconnecting) {
        handleClose();
    }
}

void TcpConnection::connectEstablished() {
    setState(kConnected);
    channel_->tie(shared_from_this()); // 强引用指针记录，防止析构
    channel_->enableReading();
    connectionCallback_(shared_from_this()); // 通知上层应用更新自身状态
}

// 调用该函数真正摧毁连接
void TcpConnection::connectDestroyed() {
    if (state_ == kConnected) {
        setState(kDisconnected);
        channel_->disableAll();
        connectionCallback_(shared_from_this()); // 通知上层应用更新自身状态
    }
    channel_->remove();
}

void TcpConnection::handleRead(Timestamp receiveTime) {
    int savedErrno = 0;

    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    // 判断读取情况
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) {
        handleClose();
    } else {
        errno = savedErrno;
        LOG_ERROR << "TcpConnection::handleRead() failed";
        handleError();
    }
}

void TcpConnection::handleWrite() {
    std::cout << "------------a" << std::endl;
    if (channel_->isWriting()) {
        int saveErrno = 0;
        ssize_t n = outputBuffer_.writeFd(channel_->fd(), &saveErrno);
        if (n > 0) {
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writecompletecallback_) {
                    loop_->queueInLoop((
                        std::bind(writecompletecallback_, shared_from_this())));
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_ERROR << "TcpConnection::handleWrite() failed";
        }
    } else {
        LOG_ERROR << "TcpConnection fd = " << channel_->fd()
                  << " is down, no more writing";
    }
}

void TcpConnection::handleClose() {
    setState(kDisconnected); // 设置连接状态
    channel_->disableAll();  // 关闭所有事件监听

    TcpConnectionPtr conPtr(shared_from_this());
    connectionCallback_(conPtr);
    closecallback_(conPtr);
}

void TcpConnection::handleError() {
    int optval;
    socklen_t optlen = sizeof(optval);
    int err = 0;
    if (::getsockopt(channel_->fd(), SOL_SOCKET, SO_ERROR, &optval, &optlen)) {
        err = errno;
    } else {
        err = optval;
    }
    LOG_ERROR << "cpConnection::handleError name:" << name_.c_str()
              << " - SO_ERROR:" << err;
}