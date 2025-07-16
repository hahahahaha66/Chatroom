#include "Codec.h"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <tuple>

std::string Codec::encode(const json& js, const std::string& type)
{
    std::string body = js.dump();
    uint32_t typelen = type.size();
    uint32_t totallen = sizeof(uint32_t) + typelen + body.size();

    std::string result;
    result.append(reinterpret_cast<const char*>(&totallen), sizeof(totallen));
    result.append(reinterpret_cast<const char*>(&typelen), sizeof(typelen));
    result.append(type);
    result.append(body);

    return result;
}

std::optional<std::tuple<std::string, json>> Codec::tryDecode(Buffer* buf)
{
    if (buf->readableBytes() < sizeof(uint32_t) * 2)
    {
        return std::nullopt;
    }

    const char* data = buf->peek();

    //读取总字数
    uint32_t totallen = 0;
    std::memcpy(&totallen, data, sizeof(uint32_t));

    if (buf->readableBytes() < sizeof(uint32_t) + totallen) 
    {
        return std::nullopt;
    }

    const char* ptr = data + sizeof(uint32_t);
    //读取类型字数
    uint32_t typelen = 0;
    std::memcpy(&typelen, ptr, sizeof(uint32_t));

    if (typelen > 256)
    {
        return std::nullopt;
    }

    ptr += sizeof(uint32_t);
    std::string type(ptr, typelen);

    ptr += typelen;
    std::string body(ptr, totallen - sizeof(uint32_t) - typelen);

    buf->retrieve(sizeof(uint32_t) + totallen);

    try {
        json js = json::parse(body);
        // LOG_INFO << "Decode message : type = " << type << ", json = " << js.dump();
        return std::make_tuple(type, js);
    } catch (...) {
        LOG_ERROR << "JSON parse failed";
        return std::nullopt;
    }
}