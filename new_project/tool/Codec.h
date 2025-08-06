#ifndef CODEC_H
#define CODEC_H

#include "../muduo/logging/Logging.h"
#include "../muduo/net/Buffer.h"
#include "../muduo/net/tcp/TcpConnection.h"

#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <optional>
#include <tuple>

using json = nlohmann::json;

class Codec {
  public:
    // 编码，添加包头
    static std::string encode(const json &js, const std::string &type);

    // 解码，解析包头
    static std::optional<std::tuple<std::string, json>> tryDecode(Buffer *buf);
};

#endif