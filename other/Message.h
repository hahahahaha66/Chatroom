#pragma one

#include "../muduo/base/Timestamp.h"

#include <string>

class Message 
{
public:
    Message(int messageid, int senderid, int receiverid, std::string content, const std::string type, const std::string status, std::string time)
        : messsageid_(messageid), senderid_(senderid), receiverid_(receiverid), content_(content), type_(type), status_(status), time_(time) {}

    Message(const Message&) = delete;
    Message& operator=(const Message&) = delete;

    Message(Message&& other) noexcept
        : messsageid_(other.messsageid_),
          senderid_(other.senderid_),
          receiverid_(other.receiverid_),
          content_(std::move(other.content_)),
          type_(std::move(other.type_)),
          status_(std::move(other.status_)),
          time_(std::move(other.time_)) 
    {
        // mutex_ 不动，保留默认状态
    }

    Message& operator=(Message&& other) noexcept 
    {
        if (this != &other) 
        {
            messsageid_ = other.messsageid_,
            senderid_ = other.senderid_,
            receiverid_ = other.receiverid_,
            content_ = std::move(other.content_),
            type_ = std::move(other.type_),
            status_ = std::move(other.status_),
            time_ = std::move(other.time_);
            // mutex_ 不赋值
        }
        return *this;
    }

    int GetId() const 
    { 
        return messsageid_; 
    }
    int GetSenderId() const 
    { 
        return senderid_; 
    }
    int GetReveiverId() const 
    { 
        return receiverid_; 
    }
    std::string GetContent() const 
    { 
        return content_; 
    }
    std::string GetType() const 
    { 
        return type_; 
    }
    std::string GetStatus() const 
    { 
        return status_; 
    }
    std::string GetTime() const 
    { 
        return time_; 
    }

    void SetStatus(std::string status) 
    { 
        status_ = status; 
    }

private:
    int messsageid_;
    int senderid_;
    int receiverid_;
    std::string content_;
    std::string type_;
    std::string status_;
    std::string time_;
};