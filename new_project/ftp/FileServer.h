#pragma once

#include "../muduo/net/tcp/TcpServer.h"
#include "../muduo/net/EventLoop.h"
#include "../tool/Codec.h"
#include "../tool/Json.h"
#include "../muduo/logging/Logging.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <unordered_map>
#include <fstream>
#include <sys/sendfile.h>
#include <cassert>
#include <ios>
#include <ostream>
#include <fcntl.h>

using json = nlohmann::json;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

struct FileTask {
    std::string filename_;
    std::string filepath_;
    int64_t filesize_ = 0;

    int fd_;  // 读取文件
    off_t offset_ = 0;  // 偏移指针

    // upload
    int64_t received_ = 0; 

    // download
    int64_t sent_ = 0;
    bool headersent_;
    bool filedatastarted_ = false;
    bool paused_ = false;  // 水位暂停

    int senderid_;
    int receiverid_;
    std::string type_;
    std::string cmd_;    // Upload or Download
};

class FileServer
{
public:
    FileServer(EventLoop* loop, const InetAddress& fileAddr, std::string name);

    void start();

    void UpLoadFile(const TcpConnectionPtr& conn, const json& js);
    void DownLoadFile(const TcpConnectionPtr& conn, const json& js);

    const std::string GetServerPort() { return server_.inPort(); }

private:
    void OnConnection(const TcpConnectionPtr&);
    void OnMessage(const TcpConnectionPtr&, Buffer*, Timestamp);
    void ThreadInitCallback(EventLoop* loop);
    void MessageCompleteCallback(const TcpConnectionPtr& conn);
    void OnHighWaterMark(const TcpConnectionPtr& conn, size_t bytespending);
    void SendFileData(const TcpConnectionPtr& conn);

    TcpServer server_;
    Codec codec_;

    std::unordered_map<TcpConnectionPtr, std::shared_ptr<FileTask>> filetasks_;
};