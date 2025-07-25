#pragma once

#include "../muduo/net/tcp/TcpServer.h"
#include "../muduo/net/EventLoop.h"
#include "../tool/Codec.h"
#include "../tool/Json.h"
#include "../muduo/logging/Logging.h"

#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>

using json = nlohmann::json;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

struct FileTask {
    std::string filename_;
    std::string filepath_;

    int64_t filesize_ = 0;
    int64_t received_ = 0;  // upload
    int64_t sent_ = 0;      // download 

    std::ofstream ofs_;  // 写入文件
    std::ifstream ifs_;  // 读取文件

    // download
    bool headersent_;
    bool filedatastarted_ = false;

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

private:
    void OnConnection(const TcpConnectionPtr&);
    void OnMessage(const TcpConnectionPtr&, Buffer*, Timestamp);
    void ThreadInitCallback(EventLoop* loop);
    void MessageCompleteCallback(const TcpConnectionPtr& conn);
    void SendFileData(const TcpConnectionPtr& conn);

    TcpServer server_;
    Codec codec_;

    std::unordered_map<TcpConnectionPtr, std::shared_ptr<FileTask>> filetasks_;
};