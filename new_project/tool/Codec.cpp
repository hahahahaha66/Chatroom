#include "Codec.h"
#include <cstdint>
#include <optional>
#include <tuple>

std::string Codec::encode(const json &js, const std::string &type) {
    std::string body = js.dump();
    uint32_t typelen = type.size();
    uint32_t totallen = sizeof(uint32_t) + typelen + body.size();

    std::string result;
    result.reserve(sizeof(totallen) + sizeof(typelen) + typelen + body.size());

    result.append(reinterpret_cast<const char *>(&totallen), sizeof(totallen));
    result.append(reinterpret_cast<const char *>(&typelen), sizeof(typelen));
    result.append(type);
    result.append(body);

    return result;
}

std::optional<std::tuple<std::string, json>> Codec::tryDecode(Buffer *buf) {
    while (true) {
        if (buf->readableBytes() < sizeof(uint32_t) * 2)
            return std::nullopt;

        const char *data = buf->peek();

        // 读取总字数
        uint32_t totallen = 0;
        std::memcpy(&totallen, data, sizeof(uint32_t));

        if (totallen < sizeof(uint32_t) || totallen > 1000 * 1024 * 1024) {
            LOG_WARN << "Invalid total length: " << totallen
                     << ", readable = " << buf->readableBytes();
            buf->retrieve(1);
            continue;
        }

        if (buf->readableBytes() < sizeof(uint32_t) + totallen)
            return std::nullopt;

        const char *ptr = data + sizeof(uint32_t);

        // 读取类型字数
        uint32_t typelen = 0;
        std::memcpy(&typelen, ptr, sizeof(uint32_t));

        if (typelen == 0 || typelen > 256) {
            LOG_WARN << "Invalid typelen: " << typelen;
            buf->retrieve(1);
            continue;
        }

        ptr += sizeof(uint32_t);
        const char *type_start = ptr;
        ptr += typelen;

        const char *json_start = ptr;
        size_t json_len = totallen - sizeof(uint32_t) - typelen;

        // 校验 json 是否超出 buf
        if ((size_t)(json_start + json_len - data) > buf->readableBytes()) {
            // 数据不够，等待更多内容
            return std::nullopt;
        }

        try {
            json js = json::parse(json_start, json_start + json_len);
            buf->retrieve(sizeof(uint32_t) + totallen);
            return std::make_tuple(std::string(type_start, typelen),
                                   std::move(js));
        } catch (const std::exception &e) {
            LOG_ERROR << "JSON parse failed" << e.what();
            buf->retrieve(1);
            return std::nullopt;
        }
    }
}