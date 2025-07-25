#pragma once

#include "../muduo/net/tcp/TcpClient.h"
#include "../muduo/net/EventLoop.h"
#include "../tool/Json.h"
#include "../tool/Codec.h"
#include "../muduo/logging/Logging.h"

#include <cstdint>
#include <fstream>
#include <string>
#include <memory>

using json = nlohmann::json;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

class FileDownloader 
{
public:
    FileDownloader(EventLoop* loop,
                   const std::string& serverip,
                   uint16_t serverport,
                   const std::string& filename,
                   const std::string& savedir);

    void Start(); 

private:
    void OnConnection(const TcpConnectionPtr& conn);
    void OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);
    void SendDownloadRequest();

    EventLoop* loop_;
    TcpClient client_;
    TcpConnectionPtr conn_;
    Codec codec_;

    std::string filename_;
    std::string savepath_;
    std::ofstream file_;

    int64_t filesize_ = 0;
    int64_t received_ = 0;
    bool gotheader_ = false;
};

class FileUploader
{
public:
    FileUploader(EventLoop* loop, const std::string& serverip,
                 uint16_t serverport, const std::string& filename,
                 const std::string& filepath, int senderid,
                 int receiverid, const std::string type);

    void Start(); 

private:
    void OnConnection(const TcpConnectionPtr& conn);
    void OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time);
    void OnWriteComplete(const TcpConnectionPtr& conn);
    void SendHeader();          
    void SendFileData();

    EventLoop* loop_;
    TcpClient client_;
    TcpConnectionPtr conn_;
    Codec codec_;

    std::ifstream file_;
    std::string filename_;
    std::string filepath_;
    int64_t filesize_ = 0;
    int64_t sent_ = 0;

    bool headersent_ = false;
    bool filedatastarted_ = false;

    int senderid_;
    int receiverid_;
    std::string type_;
};
