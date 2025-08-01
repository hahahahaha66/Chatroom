#include "FileClient.h"
#include <cstdint>

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
    fd_ = ::open(filepath_.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd_ < 0) 
    {
        LOG_ERROR << "Failed to open file: " << filepath_;
        perror("Reason");
        return;
    }
    off_t sz = ::lseek(fd_, 0, SEEK_END);
    ::lseek(fd_, 0, SEEK_SET);
    filesize_ = static_cast<int64_t>(sz);

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
    std::cout << js.dump() << std::endl;

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

    bool end = true;
    if (type == "UploadJsonBack")
    {
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
    else if (type == "AddFileToMessage")
    {
        AssignIfPresent(js, "end", end);
        if (end)
        {
            LOG_INFO << "AddFileToMessage success";
        }
        else  
        {
            LOG_ERROR << "AddFileToMessage failed";
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
    if (fd_ < 0) 
    {
        LOG_ERROR << "Invalid file descriptor";
        conn_->shutdown();
        return;
    }

    if (!filedatastarted_)
    {
        conn_->shutdown();
        ::close(fd_);
        return;
    }

    const size_t maxchunk= 128 * 1024;
    uint64_t placeholder = filesize_ / 100;
    int progress = 0;
    while (sent_ < filesize_)
    {
        off_t remaining = filesize_ - offset_;
        size_t tosend = static_cast<size_t>(std::min<off_t>(maxchunk, remaining));

        ssize_t n = ::sendfile(conn_->fd(), fd_, &offset_, tosend);

        if ((sent_ / placeholder) > progress)
        {
            progress = sent_ / placeholder;
            std::cout << "已发送大小: " << sent_ << "(" << progress << "%)" << std::endl;
        }
        
        // LOG_INFO << "Try to send total size: " << filesize_;

        if (n > 0)
        {
            sent_ = offset_;
        }
        else if (n == 0)
        {
            continue;
        }
        else  
        {
             if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 内核发送缓冲区已满，下次再继续
                continue;
            }
            // 真正错误
            LOG_ERROR << "sendfile error: " << strerror(errno);
            ::close(fd_);
            conn_->shutdown();
            return;
        }

        if (sent_ >= filesize_)
        {
            LOG_INFO << "Upload complete: " << filename_ << ",sent: " << sent_;
            ::close(fd_);
            conn_->shutdown();
        }
    }
}

FileDownloader::FileDownloader(EventLoop* loop,
                               const std::string& serverip,
                               uint16_t serverport,
                               const std::string& filename,
                               const std::string& savedir,
                               const std::string& timestamp)
    : loop_(loop),
      client_(loop, InetAddress(serverport, serverip), "FileDownloader"),
      filename_(filename),
      timestamp_(timestamp),
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
        {"filename", filename_},
        {"timestamp", timestamp_}
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

                fd_ = ::open(savepath_.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0644);
                if (fd_ < 0)
                {
                    LOG_ERROR << "Failed to open upload file";
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

        ssize_t n = ::write(fd_, data, towrite);
        if (n > 0)
        {
            received_ += n;
            buffer->retrieve(n);
            std::cout << "every write: " << n << std::endl;
            std::cout << filesize_ << " : " << received_ << std::endl;
        }
        else 
        {
            LOG_ERROR << "Write failed for file: " << filename_;
            ::close(fd_);
            conn->shutdown();
            return;
        }

        if (received_ >= filesize_) {
            LOG_INFO << "Download complete: " << filename_ << ", received: " << received_;
            ::close(fd_);
            conn->shutdown();
            return;
        }
    }
}