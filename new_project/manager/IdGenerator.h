#pragma one

#include <memory>
#include <sw/redis++/redis.h>
#include <string>
#include <sw/redis++/connection.h>
#include <mutex>
#include <shared_mutex>

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
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return redis_->incr("global:userid");
    }

    int GetNextMsgId()
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return redis_->incr("global:msgid");
    }

    int GetNextFriendId()
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return redis_->incr("global:friendid");
    }

    int GetNextGroupId()
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return redis_->incr("global:groupid");
    }

    int GetNextGroupUserId()
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return redis_->incr("global:groupuserid");
    }

    int GetNextFriendApplyId()
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return redis_->incr("global:friendapplyid");
    }

    int GetNextGroupApplyId()
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return redis_->incr("global:groupapplyid");
    }

    void InitUserId(int startid)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        redis_->set("global:userid", std::to_string(startid));
    }

    void InitMsgId(int startid)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        redis_->set("global:msgid", std::to_string(startid));
    }

    void InitFriendId(int startid)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        redis_->set("global:friendid", std::to_string(startid));
    }

    void InitGroupId(int startid)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        redis_->set("global:groupid", std::to_string(startid));
    }

    void InitGroupUserId(int startid)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        redis_->set("global:groupuserid", std::to_string(startid));
    }

    void InitFriendApplyId(int startid)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        redis_->set("global:friendapplyid", std::to_string(startid));
    }

    void InitGroupApplyId(int startid)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        redis_->set("global:groupapplyid", std::to_string(startid));
    }

    std::shared_ptr<sw::redis::Redis> GetRedis()
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return redis_;
    }

private:
    std::shared_ptr<sw::redis::Redis> redis_;
    mutable std::shared_mutex mutex_;  
};