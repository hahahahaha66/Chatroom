#pragma once

#include "../muduo/logging/Logging.h"
#include "../muduo/net/EventLoop.h"
#include "../muduo/net/tcp/TcpClient.h"
#include "../tool/Codec.h"
#include "../tool/Json.h"

#include <cstddef>
#include <cstdint>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sys/sendfile.h>
#include <sys/types.h>

using json = nlohmann::json;

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using std::placeholders::_4;

class FileUploader {
  public:
    FileUploader(EventLoop *loop, const std::string &serverip,
                 uint16_t serverport, const std::string &filename,
                 const std::string &filepath, int senderid, int receiverid,
                 const std::string type);

    void Start();

  private:
    void OnConnection(const TcpConnectionPtr &conn);
    void OnMessage(const TcpConnectionPtr &conn, Buffer *buffer,
                   Timestamp time);
    void OnWriteComplete(const TcpConnectionPtr &conn);
    void SendHeader();
    void SendFileData();

    EventLoop *loop_;
    TcpClient client_;
    TcpConnectionPtr conn_;
    Codec codec_;

    int fd_;
    off_t offset_ = 0;
    std::string filename_;
    std::string filepath_;
    int64_t filesize_ = 0;
    int64_t sent_ = 0;

    bool filedatastarted_;

    int senderid_;
    int receiverid_;
    std::string type_;
};

class FileDownloader {
  public:
    FileDownloader(EventLoop *loop, const std::string &serverip,
                   uint16_t serverport, const std::string &filename,
                   const std::string &savedir, const std::string &timestamp);

    void Start();

  private:
    void OnConnection(const TcpConnectionPtr &conn);
    void OnMessage(const TcpConnectionPtr &conn, Buffer *buffer,
                   Timestamp time);
    void SendDownloadRequest();

    EventLoop *loop_;
    TcpClient client_;
    TcpConnectionPtr conn_;
    Codec codec_;

    std::string filename_;
    std::string timestamp_;
    std::string savepath_;
    int fd_;

    int64_t filesize_;
    int64_t received_;
    bool gotheader_;
};