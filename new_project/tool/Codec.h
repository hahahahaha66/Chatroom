#pragma once
#include <cstring>
#include <optional>
#include <tuple>
#include <string>
#include <nlohmann/json.hpp>
#include <immintrin.h> // for SIMD
#include <string_view>

#include "../muduo/logging/Logging.h"
#include "../muduo/net/Buffer.h"
#include "../muduo/net/tcp/TcpConnection.h"

using json = nlohmann::json;

class Codec {
public:
    // 高效编码 - 减少内存分配和拷贝
    static std::string encode(const json &js, std::string_view type);
    
    // 高效解码 - 优化热路径
    static std::optional<std::tuple<std::string, json>> tryDecode(Buffer *buf);
    
    // 批量解码 - 一次处理多个包
    static std::vector<std::tuple<std::string, json>> tryDecodeBatch(Buffer *buf, size_t max_count = 10) {
        std::vector<std::tuple<std::string, json>> results;
        results.reserve(max_count);
        
        for (size_t i = 0; i < max_count; ++i) {
            auto result = tryDecode(buf);
            if (!result) break;
            results.emplace_back(std::move(*result));
        }
        
        return results;
    }

private:
    // 编译时常量
    static constexpr uint32_t MAX_PACKET_SIZE = 10 * 1024 * 1024; // 10MB
    static constexpr uint32_t MAX_TYPE_LEN = 256;
    static constexpr uint32_t HEADER_SIZE = sizeof(uint32_t) * 2;
    
    // 内联字节序转换 - 编译时优化
    static inline uint32_t byteswap32(uint32_t x) noexcept {
        #if defined(__GNUC__) || defined(__clang__)
            return __builtin_bswap32(x);
        #elif defined(_MSC_VER)
            return _byteswap_ulong(x);
        #else
            return ((x & 0xFF000000) >> 24) |
                   ((x & 0x00FF0000) >> 8)  |
                   ((x & 0x0000FF00) << 8)  |
                   ((x & 0x000000FF) << 24);
        #endif
    }
    
    static inline uint32_t networkToHost(uint32_t net) noexcept {
        #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
            return byteswap32(net);
        #else
            return net;
        #endif
    }
    
    static inline uint32_t hostToNetwork(uint32_t host) noexcept {
        return networkToHost(host);
    }
    
    // 快速UTF-8验证 - 使用SIMD优化
    static bool fastUTF8Check(const char* data, size_t len) noexcept {
        const unsigned char* bytes = reinterpret_cast<const unsigned char*>(data);
        size_t i = 0;
        
        // SIMD加速ASCII检查
        #ifdef __SSE2__
        for (; i + 16 <= len; i += 16) {
            __m128i chunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(bytes + i));
            if (_mm_movemask_epi8(chunk) == 0) {
                continue; // 全部是ASCII
            } else {
                // 有非ASCII字符，退出SIMD处理
                break;
            }
        }
        #endif
        
        // 处理剩余字节 - 优化的标量代码
        for (; i < len; ++i) {
            unsigned char c = bytes[i];
            if (c < 0x80) {
                continue; // ASCII
            }
            
            // 多字节UTF-8字符
            if ((c >> 5) == 0x06) { // 110xxxxx
                if (++i >= len || (bytes[i] & 0xC0) != 0x80) return false;
            } else if ((c >> 4) == 0x0E) { // 1110xxxx
                if (++i >= len || (bytes[i] & 0xC0) != 0x80) return false;
                if (++i >= len || (bytes[i] & 0xC0) != 0x80) return false;
            } else if ((c >> 3) == 0x1E) { // 11110xxx
                if (++i >= len || (bytes[i] & 0xC0) != 0x80) return false;
                if (++i >= len || (bytes[i] & 0xC0) != 0x80) return false;
                if (++i >= len || (bytes[i] & 0xC0) != 0x80) return false;
            } else {
                return false;
            }
        }
        return true;
    }
    
    // 快速包头查找 - 避免逐字节搜索
    static size_t findNextPacketHeader(const char* data, size_t len) noexcept {
        if (len < HEADER_SIZE) return len;
        
        for (size_t i = 1; i < len - HEADER_SIZE + 1; ++i) {
            uint32_t totallen_net, typelen_net;
            std::memcpy(&totallen_net, data + i, sizeof(uint32_t));
            std::memcpy(&typelen_net, data + i + sizeof(uint32_t), sizeof(uint32_t));
            
            uint32_t totallen = networkToHost(totallen_net);
            uint32_t typelen = networkToHost(typelen_net);
            
            // 快速合理性检查
            if (totallen >= sizeof(uint32_t) && totallen <= MAX_PACKET_SIZE &&
                typelen > 0 && typelen <= MAX_TYPE_LEN) {
                return i;
            }
        }
        return len;
    }
};
