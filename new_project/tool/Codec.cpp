#include "Codec.h"

std::string Codec::encode(const json &js, std::string_view type) {
    // 预先计算大小，避免字符串重新分配
    std::string body;
    body.reserve(js.dump().size()); // 预估大小
    body = js.dump();
    
    const uint32_t typelen = static_cast<uint32_t>(type.size());
    const uint32_t jsonlen = static_cast<uint32_t>(body.size());
    const uint32_t totallen = sizeof(uint32_t) + typelen + jsonlen;
    
    // 长度检查
    if (totallen > MAX_PACKET_SIZE || typelen == 0 || typelen > MAX_TYPE_LEN) {
        return {};
    }
    
    // 一次性分配内存
    std::string result;
    result.reserve(sizeof(uint32_t) * 2 + typelen + jsonlen);
    
    // 直接构建数据包
    const uint32_t totallen_net = hostToNetwork(totallen);
    const uint32_t typelen_net = hostToNetwork(typelen);
    
    result.assign(reinterpret_cast<const char*>(&totallen_net), sizeof(uint32_t));
    result.append(reinterpret_cast<const char*>(&typelen_net), sizeof(uint32_t));
    result.append(type);
    result.append(body);
    
    return result;
}

std::optional<std::tuple<std::string, json>> Codec::tryDecode(Buffer *buf) {
    // 快速路径：检查基本长度要求
    if (buf->readableBytes() < HEADER_SIZE) [[unlikely]] {
        return std::nullopt;
    }
    
    const char* data = buf->peek();
    size_t readable = buf->readableBytes();
    
    // 批量处理多个数据包的情况
    while (readable >= HEADER_SIZE) {
        // 内联读取头部信息
        uint32_t totallen_net, typelen_net;
        std::memcpy(&totallen_net, data, sizeof(uint32_t));
        std::memcpy(&typelen_net, data + sizeof(uint32_t), sizeof(uint32_t));
        
        const uint32_t totallen = networkToHost(totallen_net);
        const uint32_t typelen = networkToHost(typelen_net);
        
        // 快速边界检查
        if (totallen < sizeof(uint32_t) || totallen > MAX_PACKET_SIZE) [[unlikely]] {
            // 高效的错误恢复 - 批量跳过无效数据
            const size_t skip = findNextPacketHeader(data, readable);
            buf->retrieve(skip);
            if (skip == readable) break;
            
            data = buf->peek();
            readable = buf->readableBytes();
            continue;
        }
        
        if (typelen == 0 || typelen > MAX_TYPE_LEN) [[unlikely]] {
            buf->retrieve(1);
            data = buf->peek();
            readable = buf->readableBytes();
            continue;
        }
        
        // 检查完整数据包
        const size_t packet_size = sizeof(uint32_t) + totallen;
        if (readable < packet_size) [[unlikely]] {
            return std::nullopt; // 需要更多数据
        }
        
        // 计算偏移
        const char* type_start = data + HEADER_SIZE;
        const char* json_start = type_start + typelen;
        const size_t json_len = totallen - sizeof(uint32_t) - typelen;
        
        // 边界验证
        if (json_len == 0 || json_start + json_len > data + readable) [[unlikely]] {
            buf->retrieve(1);
            data = buf->peek();
            readable = buf->readableBytes();
            continue;
        }
        
        try {
            // 使用string_view避免不必要的拷贝
            const std::string_view json_view(json_start, json_len);
            
            // 条件性UTF-8检查 - 只在必要时检查
            static thread_local bool skip_utf8_check = false;
            if (!skip_utf8_check && !fastUTF8Check(json_start, json_len)) [[unlikely]] {
                // 首次UTF-8错误后，可能是二进制数据，暂时跳过检查
                skip_utf8_check = true;
                buf->retrieve(packet_size);
                data = buf->peek();
                readable = buf->readableBytes();
                continue;
            }
            
            // 高效JSON解析 - 直接从内存解析
            json js = json::parse(json_start, json_start + json_len, nullptr, false);
            
            // 成功解析
            buf->retrieve(packet_size);
            
            return std::make_tuple(
                std::string(type_start, typelen),
                std::move(js)
            );
            
        } catch (...) {
            // 简化异常处理 - 直接跳过问题包
            buf->retrieve(packet_size);
            data = buf->peek();
            readable = buf->readableBytes();
            continue;
        }
    }
    
    return std::nullopt;
}