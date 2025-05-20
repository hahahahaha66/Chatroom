#include "../muduo/net/tcp/TcpConnection.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class Service
{
public:
    void ProcessingMessages(const TcpConnection& conn, const json& json_str, uint16_t type, Timestamp time);
};