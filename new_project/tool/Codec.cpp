#include "Codec.h"
#include <cstdint>
#include <optional>
#include <tuple>

std::string Codec::encode(const json& js, const std::string& type)
{
    std::string body = js.dump();
    uint32_t typelen = type.size();
    uint32_t totallen = sizeof(uint32_t) + typelen + body.size();

    std::string result;
    result.reserve(sizeof(totallen) + sizeof(typelen) + typelen + body.size());

    result.append(reinterpret_cast<const char*>(&totallen), sizeof(totallen));
    result.append(reinterpret_cast<const char*>(&typelen), sizeof(typelen));
    result.append(type);
    result.append(body);

    return result;
}


std::optional<std::tuple<std::string, json>> Codec::tryDecode(Buffer* buf)
{
    if (buf->readableBytes() < sizeof(uint32_t) * 2)
        return std::nullopt;
   
    const char* data = buf->peek();

    //读取总字数
    uint32_t totallen = 0;
    std::memcpy(&totallen, data, sizeof(uint32_t));

    if (totallen < sizeof(uint32_t) || totallen > 100 * 1024 * 1024) 
    {
        LOG_WARN << "Invalid total length: " << totallen;
        return std::nullopt;
    }

    if (buf->readableBytes() < sizeof(uint32_t) + totallen) 
        return std::nullopt;

    const char* ptr = data + sizeof(uint32_t);

    //读取类型字数
    uint32_t typelen = 0;
    std::memcpy(&typelen, ptr, sizeof(uint32_t));

    if (typelen == 0 || typelen > 256)
    {
        return std::nullopt;
    }

    ptr += sizeof(uint32_t);
    const char* type_start = ptr;
    ptr += typelen;

    const char* json_start = ptr;
    size_t json_len = totallen - sizeof(uint32_t) - typelen;

    buf->retrieve(sizeof(uint32_t) + totallen);

    try {
        json js = json::parse(json_start, json_start + json_len);
        // LOG_INFO << "Decode message : type = " << type << ", json = " << js.dump();
        return std::make_tuple(std::string(type_start, typelen), std::move(js));
    } catch (...) {
        LOG_ERROR << "JSON parse failed";
        return std::nullopt;
    }
}