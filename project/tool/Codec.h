#ifndef CODEC_H
#define CODEC_H

#include "../muduo/net/Buffer.h"
#include "../muduo/net/tcp/TcpConnection.h"
#include "../muduo/logging/Logging.h"

#include <memory>
#include <nlohmann/json.hpp>
#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <tuple>

using json = nlohmann::json;

struct MessageHeader {
        size_t length;
        int type;
        int seq;
};

class Codec
{
public:
    //编码，添加包头
    static std::string encode(const json& js, int type, int seq);

    //解码，解析包头
    static std::optional<std::tuple<int, int, json>> tryDecode(Buffer* buf);
    
};

#endif