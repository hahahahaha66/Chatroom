#pragma one

#include <memory>
#include <sw/redis++/redis.h>
#include <string>
#include <sw/redis++/connection.h>
#include <mutex>

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
        std::lock_guard<std::mutex> lock(mutex_);
        return redis_->incr("global:userid");
    }

    int GetNextMsgId()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return redis_->incr("global:msgid");
    }

    int GetNextFriendId()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return redis_->incr("global:friendid");
    }

    int GetNextFriendApplyId()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return redis_->incr("global:applyid");
    }

    void InitUserId(int startid)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        redis_->set("global:userid", std::to_string(startid));
    }

    void InitMsgId(int startid)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        redis_->set("global:msgid", std::to_string(startid));
    }

    void InitFriendId(int startid)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        redis_->set("global:friendid", std::to_string(startid));
    }

    void InitFriendAppltId(int startid)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        redis_->set("global:applyid", std::to_string(startid));
    }

    std::shared_ptr<sw::redis::Redis> GetRedis()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return redis_;
    }

private:
    std::shared_ptr<sw::redis::Redis> redis_;
    std::mutex mutex_;
};