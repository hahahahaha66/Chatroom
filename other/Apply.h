#pragma one

#include <string>

class Apply 
{
public:
    Apply(int applyid, int fromid, int targetid, std::string type, std::string status)
        : applyid_(applyid), fromid_(fromid), targetid_(targetid), type_(type), status_(status) {}

    Apply(const Apply&) = delete;
    Apply& operator=(const Apply&) = delete;

    Apply(Apply&& other) noexcept
        : applyid_(other.applyid_),
          fromid_(other.fromid_),
          targetid_(other.targetid_),
          type_(std::move(other.type_)),
          status_(std::move(other.status_))
    {
        // mutex_ 不动，保留默认状态
    }

    Apply& operator=(Apply&& other) noexcept
    {
        if (this != &other)
        {
            applyid_ = other.applyid_;
            fromid_ = other.fromid_;
            targetid_ = other.targetid_;
            type_ = std::move(other.type_);
            status_ = std::move(other.status_);
        }
        return *this;
    }

    int GetApplyId() 
    { 
        return applyid_; 
    }
    int GetFromId() 
    { 
        return fromid_; 
    }
    int GetTargetId() 
    { 
        return targetid_; 
    }
    std::string GetType() 
    { 
        return type_; 
    }
    std::string GetStatus() 
    { 
        return status_; 
    }

    void SetType(std::string type) 
    { 
        type_ = type; 
    }
    void SetStatus(std::string status) 
    { 
        status_ = status; 
    }

private:
    int applyid_;
    int fromid_;
    int targetid_;
    std::string type_;
    //Pending, Agree, Reject
    std::string status_;
};