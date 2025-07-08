#pragma one

#include <string>
#include "../muduo/base/Timestamp.h"

class Message 
{
public:
    Message(int messageid, int senderid, int receiverid, std::string content, const std::string type, const std::string status, std::string time)
        : messsageid_(messageid), senderid_(senderid), receiverid_(receiverid), content_(content), type_(type), status_(status), time_(time) {}

    int GetId() const { return messsageid_; }
    int GetSenderId() const { return senderid_; }
    int GetReveiverId() const { return receiverid_; }
    std::string GetContent() const { return content_; }
    std::string GetType() const { return type_; }
    std::string GetStatus() const { return status_; }
    std::string GetTime() const { return time_; }

    void SetStatus(std::string status) { status_ = status; }

private:
    int messsageid_;
    int senderid_;
    int receiverid_;
    std::string content_;
    std::string type_;
    std::string status_;
    std::string time_;
};