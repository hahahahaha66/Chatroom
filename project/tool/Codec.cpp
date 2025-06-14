#include "Codec.h"
#include <cstdint>
#include <optional>
#include <tuple>

std::string Codec::encode(const json& js, int type, int seq)
{
    std::string body = js.dump();
    uint32_t len = sizeof(MessageHeader) + body.size();

    MessageHeader header{len, type, seq};

    std::string result;
    result.append(reinterpret_cast<const char*>(&header), sizeof(header));
    result.append(body);
    return result;
}

std::optional<std::tuple<int, int, json>> Codec::tryDecode(Buffer* buf)
{
    if (buf->readableBytes() < sizeof(MessageHeader))
    {
        return std::nullopt;
    }

    const char* data = buf->peek();
    const MessageHeader* hdr = reinterpret_cast<const MessageHeader*>(data);

    if (buf->readableBytes() < hdr->length) {
        return std::nullopt;
    }

    std::string body(data + sizeof(MessageHeader), hdr->length - sizeof(MessageHeader));
    buf->retrieve(hdr->length);

    try {
        json js = json::parse(body);

        LOG_INFO << js.dump();

        return std::make_tuple(hdr->type, hdr->seq, js);
    } catch (...) {
        return std::nullopt;
    }
}