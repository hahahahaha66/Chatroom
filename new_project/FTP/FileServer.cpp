#include "FileServer.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <memory>
#include <ostream>
#include <thread>
#include <unistd.h>

FileServer::FileServer(EventLoop* loop, const InetAddress& fileAddr, std::string name)
    : server_(loop, fileAddr, name)
{
    server_.setThreadNum(8);

    server_.setConnectionCallback(std::bind(&FileServer::OnConnection, this, _1));
    server_.setMessageCallback(std::bind(&FileServer::OnMessage, this, _1, _2, _3));
    server_.setWriteCompleteCallback(std::bind(&FileServer::MessageCompleteCallback, this, _1));
    server_.setThreadInitCallback(std::bind(&FileServer::ThreadInitCallback, this, _1));
}

void FileServer::start()
{
    server_.start();
}

void FileServer::ThreadInitCallback(EventLoop* loop)
{
    
}

void FileServer::MessageCompleteCallback(const TcpConnectionPtr& conn)
{
   auto it = filetasks_.find(conn);
   if (it != filetasks_.end() && it->second->cmd_ == "Download")
   {
        auto task = it->second;
        if (task->filedatastarted_ && task->sent_ < task->filesize_)
        {
            SendFileData(conn);
        }
   }
}

void FileServer::OnConnection(const TcpConnectionPtr& conn) 
{
    if (conn->connected()) 
    {
        LOG_INFO << "New connection from " << conn->peerAddress().toIpPort();
    } 
    else
    {
        LOG_INFO << "Connection disconnected: " << conn->peerAddress().toIpPort();
        if (filetasks_.find(conn) != filetasks_.end())
        {
            filetasks_.erase(conn);
        }
    }
}

void FileServer::OnMessage(const TcpConnectionPtr& conn, Buffer* buffer, Timestamp time) 
{
    auto it = filetasks_.find(conn);

    if (it == filetasks_.end())
    {
        auto msgopt = codec_.tryDecode(buffer);
        if (!msgopt.has_value())
        {
            LOG_INFO << "Recv from " << conn->peerAddress().toIpPort() << " failed";
            return ;
        }
        auto [type, js] = msgopt.value();

        if (type == "Upload")
        {
            UpLoadFile(conn, js);
        }
        else if (type == "Download")
        {
            DownLoadFile(conn, js);
        }
        else  
        {
            LOG_ERROR << "Unknown type command: " << type;
            conn->shutdown();
            return;
        }
    }
    else  
    {
        auto task = it->second;
        const char* data = buffer->peek();
        size_t len = buffer->readableBytes();

        if (len == 0) return;

        size_t remain = task->filesize_ - task->received_;
        size_t towrite = std::min(len, remain);

        task->ofs_.write(data, towrite);
        if (task->ofs_.fail())
        {
            LOG_ERROR << "Write failed for file: " << task->filename_;
            task->ofs_.close();
            conn->shutdown();
            filetasks_.erase(conn);
            return;
        }
        task->received_ += towrite;
        buffer->retrieve(towrite);

        std::cout << task->filesize_ << " : " << task->received_ << std::endl;

        if (task->received_ >= task->filesize_)
        {
            LOG_INFO << "Upload complete: " << task->filename_;
            task->ofs_.close();
            conn->shutdown();

            // 这里通知主服务器加载完成，可以写入消息并转发

            filetasks_.erase(conn);
        }
    }
}

void FileServer::UpLoadFile(const TcpConnectionPtr& conn, const json& js)
{
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

    if (!end)
    {
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

    std::string filepath = "/home/hahaha/work/Chatroom/new_project/FTP/files/" + filename;
    task->ofs_.open(filepath, std::ios::binary);
    if (!task->ofs_.is_open())
    {
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

    json j = {
        {"end", true}
    };
    
    conn->send(codec_.encode(j, "UploadJsonBack"));
    return;
}

void FileServer::DownLoadFile(const TcpConnectionPtr& conn, const json& js)
{
    bool end = true;
    std::string filename;
    std::streamsize filesize = 0;
    if (!AssignIfPresent(js, "filename", filename))
    {
        LOG_ERROR << "Missing filename in download request";
        end = false;
    }

    auto task = std::make_shared<FileTask>();
    std::string filepath = "/home/hahaha/work/Chatroom/new_project/FTP/files/" + filename;
    if (end)
    {
        task->ifs_.open(filepath, std::ios::binary);
        if (!task->ifs_.is_open())
        {
            LOG_ERROR << "Request file not found: " << filename;
            end = false;
        }
        else  
        {
            task->ifs_.seekg(0, std::ios::end);
            filesize = task->ifs_.tellg();
            task->ifs_.seekg(0);
        }
    }

    task->filename_ = filename;
    task->filesize_ = filesize;
    task->filepath_ = filepath;
    task->cmd_ = "Download";
    filetasks_[conn] = task;

    json j;
    if (end)
    {
        j = {
            {"end", true},
            {"filename", filename},
            {"filesize", filesize}
        };
    }
    else  
    {
        j = {
            {"end", false}
        };
    }

    conn->send(codec_.encode(j, "DownloadBack"));
    if (end) 
    {
        sleep(1);
        task->filedatastarted_ = true;
        SendFileData(conn);
    }
}

void FileServer::SendFileData(const TcpConnectionPtr& conn)
{
    auto it = filetasks_.find(conn);
    if (it == filetasks_.end())
    {
        LOG_ERROR << "Failed to find filetask data";
        conn->shutdown();
        return;
    }
    else  
    {
        auto task = it->second;
        if (task->ifs_.eof() || task->sent_ >= task->filesize_)
        {
            if (task->sent_ >= task->filesize_)
            {
                LOG_INFO << "Download complete: " << task->filename_ << ", sent: " << task->sent_;
            }
            task->ifs_.close();
            conn->shutdown();
            filetasks_.erase(conn);
            return;
        }

        const size_t bufferSize = 8 * 1024; 
        char buffer[bufferSize];

        task->ifs_.read(buffer, bufferSize);
        std::streamsize readbytes = task->ifs_.gcount();

        if (readbytes > 0) 
        {
            if (task->sent_ + readbytes > task->filesize_)
            {
                readbytes = task->filesize_ - task->sent_;
            }

            conn->send(std::string(buffer, readbytes));
            task->sent_ += readbytes;
            std::cout << task->filesize_  << " : " << task->sent_ << std::endl;
        }
        else 
        {
            LOG_ERROR << "Failed to read file data";
            task->ifs_.close();
            conn->shutdown();
            filetasks_.erase(conn);
        }
    }
}