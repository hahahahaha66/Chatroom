#pragma one

#include <string>

class Apply 
{
public:
    Apply(int applyid, int fromid, int targetid, std::string type, std::string status)
        : applyid_(applyid), fromid_(fromid), targetid_(targetid), type_(type), status_(status)
    {
    }

    int GetApplyId() { return applyid_; }
    int GetFromId() { return fromid_; }
    int GetTargetId() { return targetid_; }
    std::string GetType() { return type_; }
    std::string GetStatus() { return status_; }

    void SetType(std::string type) { type_ = type; }
    void SetStatus(std::string status) { status_ = status; }

private:
    int applyid_;
    int fromid_;
    int targetid_;
    std::string type_;
    std::string status_;
};