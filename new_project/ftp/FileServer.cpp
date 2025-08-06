#include "FileServer.h"
#include <fstream>
#include <memory>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string>

FileServer::FileServer(EventLoop *loop, const InetAddress &fileAddr,
                       std::string name)
    : server_(loop, fileAddr, name), loop_(loop) {
    server_.setThreadNum(8);
    server_.setConnectionCallback(
        std::bind(&FileServer::OnConnection, this, _1));
    server_.setMessageCallback(
        std::bind(&FileServer::OnMessage, this, _1, _2, _3));
    server_.setWriteCompleteCallback(
        std::bind(&FileServer::MessageCompleteCallback, this, _1));
    server_.setThreadInitCallback(
        std::bind(&FileServer::ThreadInitCallback, this, _1));
    server_.setHighWaterMarkCallback(
        std::bind(&FileServer::OnHighWaterMark, this, _1, _2), 1 * 1024 * 1024);

    InetAddress addr(10101, "10.30.0.120");
    client_ = std::make_unique<TcpClient>(loop_, addr, "serverclient");
    client_->setMessageCallback(
        [this](const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time) {
            auto msgopt = codec_.tryDecode(buffer);
            if (!msgopt.has_value()) {
                LOG_INFO << "Recv from " << conn->peerAddress().toIpPort()
                         << " failed";
                return;
            }
            auto [type, js] = msgopt.value();
            if (type == "AddFileToMessageBack") {
                std::lock_guard<std::mutex> lock(mutex_);
                AssignIfPresent(js, "end", msgsend);
                waitingback_ = false;
            }
            cv_.notify_one();
        });

    client_->connect();
}

void FileServer::start() { server_.start(); }

void FileServer::ThreadInitCallback(EventLoop *loop) {}

void FileServer::MessageCompleteCallback(const TcpConnectionPtr &conn) {
    auto it = filetasks_.find(conn);
    if (it != filetasks_.end() && it->second->cmd_ == "Download") {
        auto task = it->second;
        task->paused_ = false;

        SendFileData(conn);

        // 如果发送完毕，再关闭
        if (task->sent_ >= task->filesize_) {
            LOG_INFO << "[Server] Download complete: " << task->filename_;
            ::close(task->fd_);
            conn->shutdown();
            filetasks_.erase(conn);
        }
    }
}

void FileServer::OnHighWaterMark(const TcpConnectionPtr &conn,
                                 size_t bytespending) {
    LOG_WARN << "[Server] HighWaterMark: pending=" << bytespending;
    // 在这里可以记录一个标志，告诉 SendFileData 暂停读文件
    auto it = filetasks_.find(conn);
    if (it != filetasks_.end()) {
        it->second->paused_ = true;
    }
}

void FileServer::OnConnection(const TcpConnectionPtr &conn) {
    if (conn->connected()) {
        LOG_INFO << "New connection from " << conn->peerAddress().toIpPort();
    } else {
        LOG_INFO << "Connection disconnected: "
                 << conn->peerAddress().toIpPort();
        if (filetasks_.find(conn) != filetasks_.end()) {
            filetasks_.erase(conn);
        }
    }
}

void FileServer::OnMessage(const TcpConnectionPtr &conn, Buffer *buffer,
                           Timestamp time) {
    auto it = filetasks_.find(conn);

    if (it == filetasks_.end()) {
        auto msgopt = codec_.tryDecode(buffer);
        if (!msgopt.has_value()) {
            LOG_INFO << "Recv from " << conn->peerAddress().toIpPort()
                     << " failed";
            return;
        }
        auto [type, js] = msgopt.value();

        if (type == "Upload") {
            UpLoadFile(conn, js);
        } else if (type == "Download") {
            DownLoadFile(conn, js);
        } else {
            LOG_ERROR << "Unknown type command: " << type;
            conn->shutdown();
            return;
        }
    } else {
        auto task = it->second;

        if (task->cmd_ == "Upload") {
            const char *data = buffer->peek();
            size_t len = buffer->readableBytes();

            if (len == 0)
                return;

            size_t remain = task->filesize_ - task->received_;
            size_t towrite = std::min(len, remain);

            ssize_t n = ::write(task->fd_, data, towrite);
            if (n > 0) {
                task->received_ += n;
                buffer->retrieve(n);
                // std::cout << task->filesize_ << " : " << task->received_ <<
                // std::endl;
            } else {
                LOG_ERROR << "Write failed for file: " << task->filename_;
                ::close(task->fd_);
                conn->shutdown();
                filetasks_.erase(conn);
                return;
            }

            if (task->received_ >= task->filesize_) {
                LOG_INFO << "Upload complete: " << task->filename_;
                ::close(task->fd_);
                std::string filehash = ComputeSHA256(task->filepath_);
                RenameFileToHash(
                    task->filepath_, filehash,
                    "/home/hahaha/work/Chatroom/new_project/ftp/files/");

                json js = {{"senderid", task->senderid_},
                           {"receiverid", task->receiverid_},
                           {"type", task->type_},
                           {"filename", task->filename_},
                           {"filehash", filehash},
                           {"filesize", task->filesize_}};
                filetasks_.erase(conn);

                waitingback_ = true;
                NotifyMainServer(codec_.encode(js, "AddFileToMessage"));
                waitInPutReady();
                js = {{"end", msgsend}};
                conn->send(codec_.encode(js, "AddFileToMessage"));

                conn->shutdown();
            }
        }
    }
}

inline void FileServer::waitInPutReady() {
    if (waitingback_) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !waitingback_; });
    }
}

std::string FileServer::ComputeSHA256(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        return "";
    }

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (!ctx)
        return "";

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    char buffer[8192];
    while (file.good()) {
        file.read(buffer, sizeof(buffer));
        if (EVP_DigestUpdate(ctx, buffer, file.gcount()) != 1) {
            EVP_MD_CTX_free(ctx);
            return "";
        }
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int len = 0;
    if (EVP_DigestFinal_ex(ctx, hash, &len) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    EVP_MD_CTX_free(ctx);

    std::ostringstream oss;
    for (unsigned int i = 0; i < len; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return oss.str();
}

bool FileServer::RenameFileToHash(const std::string &oldpath,
                                  const std::string &hash,
                                  const std::string &dir) {
    std::string newpath = dir + "/" + hash;
    return std::rename(oldpath.c_str(), newpath.c_str()) == 0;
}

void FileServer::UpLoadFile(const TcpConnectionPtr &conn, const json &js) {
    bool end = true;
    std::string filename;
    int64_t filesize;
    end &= AssignIfPresent(js, "filename", filename);
    end &= AssignIfPresent(js, "filesize", filesize);

    int senderid = -1;
    int receiverid = -1;
    std::string type;
    AssignIfPresent(js, "senderid", senderid);
    AssignIfPresent(js, "receiverid", receiverid);
    AssignIfPresent(js, "type", type);

    if (!end) {
        LOG_ERROR << "Missing filename or filesize in upload request";
        json j = {{"end", false}};
        conn->send(codec_.encode(j, "UploadJsonBack"));
        conn->shutdown();
        return;
    }

    if (filesize <= 0 || filesize > (10LL << 30)) // 限制最大10GB
    {
        LOG_ERROR << "Illegal file size: " << filesize;
        conn->shutdown();
        return;
    }

    auto task = std::make_shared<FileTask>();
    task->filename_ = filename;
    task->filesize_ = filesize;
    task->received_ = 0;

    std::string filepath =
        "/home/hahaha/work/Chatroom/new_project/ftp/files/" + filename;
    task->fd_ = ::open(filepath.c_str(),
                       O_WRONLY | O_CREAT | O_TRUNC | O_NONBLOCK, 0644);
    if (task->fd_ < 0) {
        LOG_ERROR << "Failed to open upload file";
        conn->shutdown();
        return;
    }

    task->filepath_ = filepath;
    task->cmd_ = "Upload";
    task->senderid_ = senderid;
    task->receiverid_ = receiverid;
    task->type_ = type;

    filetasks_[conn] = task;

    json j = {{"end", true}};

    conn->send(codec_.encode(j, "UploadJsonBack"));
    return;
}

void FileServer::DownLoadFile(const TcpConnectionPtr &conn, const json &js) {
    bool end = true;
    std::string filename;
    std::string filehash;
    std::streamsize filesize = 0;
    std::string timestamp;
    end &= AssignIfPresent(js, "filename", filename);
    end &= AssignIfPresent(js, "timestamp", timestamp);

    if (end) {
        MysqlConnPtr conn = MysqlConnectionPool::Instance().GetConnection();
        if (conn->IsConnected()) {
            std::string query =
                "select filehash from files where filename = '" + filename +
                "' and timestamp = '" + timestamp + "' limit 1 ; ";
            MYSQL_RES *res = conn->ExcuteQuery(query);
            if (!res) {
                LOG_ERROR << "No files found with name: " << filename
                          << " query failed ";
                return;
            }
            MysqlResult result(res);
            if (!result.Next()) {
                LOG_ERROR << "No files found with name: " << filename;
                return;
            }
            filehash = result.GetRow().GetString("filehash");
        }
    }

    auto task = std::make_shared<FileTask>();
    std::string filepath =
        "/home/hahaha/work/Chatroom/new_project/ftp/files/" + filehash;
    if (end) {
        task->fd_ = ::open(filepath.c_str(), O_RDONLY | O_NONBLOCK);
        std::cout << filepath << std::endl;
        if (task->fd_ < 0) {
            LOG_ERROR << "Request file not found: " << filename;
            end = false;
        } else {
            off_t sz = ::lseek(task->fd_, 0, SEEK_END);
            ::lseek(task->fd_, 0, SEEK_SET);
            filesize = static_cast<int64_t>(sz);
        }
    }

    task->filename_ = filename;
    task->filesize_ = filesize;
    task->filepath_ = filepath;
    task->offset_ = 0;
    task->cmd_ = "Download";
    filetasks_[conn] = task;
    task->sent_ = 0;
    task->filedatastarted_ = end;

    json j;
    if (end) {
        j = {{"end", true}, {"filename", filename}, {"filesize", filesize}};
    } else {
        j = {{"end", false}};
    }

    conn->send(codec_.encode(j, "DownloadBack"));
    if (end) {
        task->filedatastarted_ = true;
        SendFileData(conn);
    }
}

void FileServer::SendFileData(const TcpConnectionPtr &conn) {
    auto it = filetasks_.find(conn);
    if (it == filetasks_.end()) {
        LOG_ERROR << "Failed to find filetask data";
        conn->shutdown();
        return;
    }

    auto task = it->second;

    if (!task->filedatastarted_) {
        conn->shutdown();
        filetasks_.erase(conn);
        ::close(task->fd_);
        return;
    }

    if (task->paused_)
        return;

    const size_t maxchunk = 128 * 1024;
    while (task->sent_ < task->filesize_) {
        off_t remaining = task->filesize_ - task->offset_;
        size_t tosend =
            static_cast<size_t>(std::min<off_t>(maxchunk, remaining));

        ssize_t n = ::sendfile(conn->fd(), task->fd_, &task->offset_, tosend);

        if (n > 0) {
            task->sent_ = task->offset_;
            std::cout << task->filesize_ << " : " << task->sent_ << std::endl;
        } else if (n == 0) {
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 内核发送缓冲区已满，下次再继续
                return;
            }
            // 真正错误
            LOG_ERROR << "sendfile error: " << strerror(errno);
            ::close(task->fd_);
            conn->shutdown();
            filetasks_.erase(conn);
            return;
        }

        if (conn->getOutputBuffer().readableBytes() >=
            conn->getHighWaterMark()) {
            LOG_INFO << "[Server] Pausing send: bufferSize="
                     << conn->getOutputBuffer().readableBytes();
            task->paused_ = true;
            break;
        }
    }

    if (task->sent_ >= task->filesize_) {
        LOG_INFO << "Download complete: " << task->filename_
                 << ", sent: " << task->sent_;
    }
}

void FileServer::NotifyMainServer(const std::string messages) {
    loop_->runInLoop([this, messages]() {
        if (client_->connection() && client_->connection()->connected()) {
            client_->connection()->send(messages);
        } else {
            LOG_ERROR << "main server not conected";
        }
    });
}