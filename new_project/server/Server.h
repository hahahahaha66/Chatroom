#pragma once
#include "../ftp/FileServer.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/TcpServer.h"
#include "../tool/Codec.h"
#include "../tool/Dispatcher.h"
#include "Service.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

class Server {
  public:
    Server(EventLoop *loop, const InetAddress &listenAddr,
           const InetAddress &fileAddr, std::string name);

    void start();

  private:
    void OnConnection(const TcpConnectionPtr &);
    void OnMessage(const TcpConnectionPtr &, Buffer *, Timestamp);
    void ThreadInitCallback(EventLoop *loop);
    void MessageCompleteCallback(const TcpConnectionPtr &conn);

    template <typename Func>
    void RegisterHandlerSafe(Dispatcher &dispatcher, const std::string &type,
                             Service &service, Func f) {
        dispatcher.registerHandler(type,
                                   std::bind(f, std::ref(service), _1, _2));
    }

    TcpServer server_;
    FileServer fileserver_;
    Codec codec_;
    Dispatcher dispatcher_;

    Service service_;
};