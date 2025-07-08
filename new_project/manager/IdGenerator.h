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

    void InitUserId(int startId)
    {
        redis_->set("global:userid", std::to_string(startId));
    }

    void InitMsgId(int startId)
    {
        redis_->set("global:msgid", std::to_string(startId));
    }

    std::shared_ptr<sw::redis::Redis> GetRedis()
    {
        return redis_;
    }

private:
    std::shared_ptr<sw::redis::Redis> redis_;
};