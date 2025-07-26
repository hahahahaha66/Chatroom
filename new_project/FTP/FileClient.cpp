#include "FileClient.h"

#include <cstddef>
#include <cstdint>
#include <iostream>

FileUploader::FileUploader(EventLoop* loop, const std::string& serverip,
                 uint16_t serverport, const std::string& filename,
                 const std::string& filepath, int senderid,
                 int receiverid, const std::string type)
    : loop_(loop),
      client_(loop, InetAddress(serverport, serverip), "FileUploader"),
      filename_(filename),
      filepath_(filepath + '/' + filename),
      senderid_(senderid),
      receiverid_(receiverid),
      type_(type),
      sent_(0),
      filedatastarted_(false)
{
    file_.open(filepath_, std::ios::binary | std::ios::in);
    if (!file_.is_open()) 
    {
        LOG_ERROR << "Failed to open file: " << filepath_;
        return;
    }
    file_.seekg(0, std::ios::end);
    filesize_ = file_.tellg();
    file_.seekg(0);

    client_.setConnectionCallback(std::bind(&FileUploader::OnConnection, this, _1));
    client_.setMessageCallback(std::bind(&FileUploader::OnMessage, this, _1, _2, _3));
    client_.setWriteCompleteCallback(std::bind(&FileUploader::OnWriteComplete, this, _1));
}

void FileUploader::Start()
{
    client_.connect();
}

void FileUploader::OnConnection(const TcpConnectionPtr& conn) {
    if (conn->connected()) {
        conn_ = conn;
        SendHeader();  
    } else {
        std::cout << "Disconnected from file server" << std::endl;
    }
}

void FileUploader::SendHeader()
{
    json js {
        {"filename", filename_},
        {"filesize", filesize_},
        {"senderid", senderid_},
        {"receiverid", receiverid_},
        {"type", type_}
    };
    std::string msg = codec_.encode(js, "Upload");

    if (conn_ && conn_->connected())
    {
        conn_->send(msg);
        LOG_INFO << "Header sent for file: " << filename_ << ", size: " << filesize_;
    }
}

void FileUploader::OnWriteComplete(const TcpConnectionPtr& conn) 
{

    if (filedatastarted_ && sent_ < filesize_)
    {
        SendFileData();
    }
}

void FileUploader::OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time)
{
    auto msgopt = codec_.tryDecode(buffer);
    if (!msgopt.has_value())
    {
        LOG_INFO << "Recv from " << conn->peerAddress().toIpPort() << " failed";
        return;
    }
    auto [type, js] = msgopt.value();

    if (type == "UploadJsonBack")
    {
        bool end = true;
        AssignIfPresent(js, "end", end);
        if (end)
        {
            LOG_INFO << "UploadJsonBack success";
            if (!filedatastarted_) 
            {
                filedatastarted_ = true;
                SendFileData();
            }
        }
        else  
        {
            LOG_ERROR << "UploadJsonBack failed";
            conn->shutdown();
        }
    }
    else  
    {
        LOG_ERROR << "Woring Type";
        conn->shutdown();
    }
}

void FileUploader::SendFileData() 
{
    if (file_.eof() || sent_ >= filesize_)
    {
        if (sent_ >= filesize_)
        {
            LOG_INFO << "Upload complete: " << filename_ << ", sent: " << sent_;
        }
        file_.close();
        conn_->shutdown();
        return;
    }

    while (sent_ < filesize_ && !file_.eof())
    {
        const size_t bufferSize = 8 * 1024;  // 64KB
        char buffer[bufferSize];

        file_.read(buffer, bufferSize);
        std::streamsize readbytes = file_.gcount();

        if (readbytes > 0) 
        {
            if (sent_ + readbytes > filesize_)
            {
                readbytes = filesize_ - sent_;
            }

            conn_->send(std::string(buffer, readbytes));
            sent_ += readbytes;
            std::cout << filesize_  << " : " << sent_ << std::endl;

            if (sent_ >= filesize_)
            {
                LOG_INFO << "Upload complete: " << filename_ << ",sent: " << sent_;
                file_.close();
                conn_->shutdown();
            }
        }
        else 
        {
            LOG_ERROR << "Failed to read file data";
            file_.close();
            conn_->shutdown();
        }
    }
}

FileDownloader::FileDownloader(EventLoop* loop,
                               const std::string& serverip,
                               uint16_t serverport,
                               const std::string& filename,
                               const std::string& savedir)
    : loop_(loop),
      client_(loop, InetAddress(serverport, serverip), "FileDownloader"),
      filename_(filename),
      savepath_(savedir + "/" + filename),
      filesize_(0),
      received_(0),
      gotheader_(false)
{
    client_.setConnectionCallback(std::bind(&FileDownloader::OnConnection, this, _1));
    client_.setMessageCallback(std::bind(&FileDownloader::OnMessage, this, _1, _2, _3));
    client_.setWriteCompleteCallback([](const TcpConnectionPtr conn) {});
}

void FileDownloader::Start() 
{
    client_.connect();
}

void FileDownloader::OnConnection(const TcpConnectionPtr& conn) 
{
    if (conn->connected()) {
        conn_ = conn;
        SendDownloadRequest();
    } else {
        LOG_INFO << "Disconnected from file server";
    }
}

void FileDownloader::SendDownloadRequest() 
{
    json js = {
        {"filename", filename_}
    };
    std::string msg = codec_.encode(js, "Download");

    if (conn_ && conn_->connected())
    {
        conn_->send(msg);
    }
}

void FileDownloader::OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time) 
{
    if (!gotheader_)
    {
        auto msgopt = codec_.tryDecode(buffer);
        if (!msgopt.has_value())
        {
            LOG_INFO << "Recv from " << conn->peerAddress().toIpPort() << " failed";
            return;
        }
        auto [type, js] = msgopt.value();

        if (type == "DownloadBack")
        {
            bool end = true;
            end &= AssignIfPresent(js, "end", end);
            if (end)
            {
                AssignIfPresent(js, "filename", filename_);
                AssignIfPresent(js, "filesize", filesize_);
                gotheader_ = true;

                file_.open(savepath_, std::ios::binary);
                if (!file_.is_open()) {
                    LOG_ERROR << "Failed to open file for writing: " << savepath_;
                    conn->shutdown();
                    return;
                }
                std::cout << "Start receiving file: " << filename_ << ", size: " << filesize_ << std::endl;
                LOG_INFO << "DownloadJsonBack success";
            }
            else  
            {
                LOG_ERROR << "DownloadJsonBack failed";
                conn->shutdown();
                return;
            }
        }
        else  
        {
            LOG_ERROR << "Wrong Type: " << type;
            conn->shutdown();
            return;
        }
    }

    // 正式读取文件内容
    while (gotheader_ && buffer->readableBytes() > 0 && received_ < filesize_)
    {
        const char* data = buffer->peek();
        size_t len = buffer->readableBytes();

        if (len == 0) return;

        size_t remaining = filesize_ - received_;
        size_t towrite = std::min(len, remaining);

        file_.write(data, towrite);
        if (file_.fail())
        {
            LOG_ERROR << "Write failed for file: " << filename_;
            file_.close();
            conn->shutdown();
            return;
        }
        std::cout << "every write: " << towrite << std::endl;
        received_ += towrite;
        buffer->retrieve(towrite);
        
        std::cout << filesize_ << " : " << received_ << std::endl;

        if (received_ >= filesize_) {
            std::cout << "Download complete: " << filename_ << std::endl;
            LOG_INFO << "Download complete: " << filename_ << ", received: " << received_;
            file_.close();
            conn->shutdown();
            return;
        }
    }
}