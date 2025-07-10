#pragma one

#include <memory>
#include <sw/redis++/redis.h>
#include <string>
#include <sw/redis++/connection.h>

class IdGenerator 
{
public:
    IdGenerator()
    {
        sw::redis::ConnectionOptions opts;
        opts.host = "127.0.0.1";
        opts.port = 6379;

        redis_ = std::make_shared<sw::redis::Redis>(opts);
    }

    int GetNextUserId()
    {
        return redis_->incr("global:userid");
    }

    int GetNextMsgId()
    {
        return redis_->incr("global:msgid");
    }

    int GetNextFriendId()
    {
        return redis_->incr("global:friendid");
    }

    int GetNextFriendApplyId()
    {
        return redis_->incr("global:applyid");
    }

    void InitUserId(int startid)
    {
        redis_->set("global:userid", std::to_string(startid));
    }

    void InitMsgId(int startid)
    {
        redis_->set("global:msgid", std::to_string(startid));
    }

    void InitFriendId(int startid)
    {
        redis_->set("global:friendid", std::to_string(startid));
    }

    void InitFriendAppltId(int startid)
    {
        redis_->set("global:applyid", std::to_string(startid));
    }

    std::shared_ptr<sw::redis::Redis> GetRedis()
    {
        return redis_;
    }

private:
    std::shared_ptr<sw::redis::Redis> redis_;
};