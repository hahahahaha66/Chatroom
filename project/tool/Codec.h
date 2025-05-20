#ifndef CODEC_H
#define CODEC_H

#include "../muduo/net/Buffer.h"
#include "../muduo/net/tcp/TcpConnection.h"

#include <memory>
#include <nlohmann/json.hpp>
#include <cstdint>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <tuple>

using json = nlohmann::json;

struct MessageHeader {
        size_t length;
        uint16_t type;
        uint16_t seq;
};

class Codec
{
public:
    //编码，添加包头
    static std::string encode(const json& js, uint16_t type, uint16_t seq = 0);

    //解码，解析包头
    static std::optional<std::tuple<uint16_t, uint16_t, json>> tryDecode(Buffer* buf);
    
};

#endif