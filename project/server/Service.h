#include "../muduo/net/tcp/TcpConnection.h"
#include "../tool/Dispatcher.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Service
{
public:
    Service();
    ~Service();

    void ProcessingMessages(const TcpConnectionPtr& conn, const json& json_str, Timestamp time);

private:
    Dispatcher dispatcher_;
};