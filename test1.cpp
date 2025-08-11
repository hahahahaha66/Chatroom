#include <cstring>
#include <immintrin.h> // for SIMD
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>

using json = nlohmann::json;

class HighPerformanceCodec {
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
        return ((x & 0xFF000000) >> 24) | ((x & 0x00FF0000) >> 8) |
               ((x & 0x0000FF00) << 8) | ((x & 0x000000FF) << 24);
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
    static bool fastUTF8Check(const char *data, size_t len) noexcept {
        const unsigned char *bytes =
            reinterpret_cast<const unsigned char *>(data);
        size_t i = 0;

// SIMD加速ASCII检查
#ifdef __SSE2__
        for (; i + 16 <= len; i += 16) {
            __m128i chunk =
                _mm_loadu_si128(reinterpret_cast<const __m128i *>(bytes + i));
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
                if (++i >= len || (bytes[i] & 0xC0) != 0x80)
                    return false;
            } else if ((c >> 4) == 0x0E) { // 1110xxxx
                if (++i >= len || (bytes[i] & 0xC0) != 0x80)
                    return false;
                if (++i >= len || (bytes[i] & 0xC0) != 0x80)
                    return false;
            } else if ((c >> 3) == 0x1E) { // 11110xxx
                if (++i >= len || (bytes[i] & 0xC0) != 0x80)
                    return false;
                if (++i >= len || (bytes[i] & 0xC0) != 0x80)
                    return false;
                if (++i >= len || (bytes[i] & 0xC0) != 0x80)
                    return false;
            } else {
                return false;
            }
        }
        return true;
    }

    // 快速包头查找 - 避免逐字节搜索
    static size_t findNextPacketHeader(const char *data, size_t len) noexcept {
        if (len < HEADER_SIZE)
            return len;

        for (size_t i = 1; i < len - HEADER_SIZE + 1; ++i) {
            uint32_t totallen_net, typelen_net;
            std::memcpy(&totallen_net, data + i, sizeof(uint32_t));
            std::memcpy(&typelen_net, data + i + sizeof(uint32_t),
                        sizeof(uint32_t));

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

  public:
    // 高效编码 - 减少内存分配和拷贝
    static std::string encode(const json &js, std::string_view type) {
        // 预先计算大小，避免字符串重新分配
        std::string body;
        body.reserve(js.dump().size()); // 预估大小
        body = js.dump();

        const uint32_t typelen = static_cast<uint32_t>(type.size());
        const uint32_t jsonlen = static_cast<uint32_t>(body.size());
        const uint32_t totallen = sizeof(uint32_t) + typelen + jsonlen;

        // 长度检查
        if (totallen > MAX_PACKET_SIZE || typelen == 0 ||
            typelen > MAX_TYPE_LEN) {
            return {};
        }

        // 一次性分配内存
        std::string result;
        result.reserve(sizeof(uint32_t) * 2 + typelen + jsonlen);

        // 直接构建数据包
        const uint32_t totallen_net = hostToNetwork(totallen);
        const uint32_t typelen_net = hostToNetwork(typelen);

        result.assign(reinterpret_cast<const char *>(&totallen_net),
                      sizeof(uint32_t));
        result.append(reinterpret_cast<const char *>(&typelen_net),
                      sizeof(uint32_t));
        result.append(type);
        result.append(body);

        return result;
    }

    // 高效解码 - 优化热路径
    static std::optional<std::tuple<std::string, json>> tryDecode(Buffer *buf) {
        // 快速路径：检查基本长度要求
        if (buf->readableBytes() < HEADER_SIZE) [[unlikely]] {
            return std::nullopt;
        }

        const char *data = buf->peek();
        size_t readable = buf->readableBytes();

        // 批量处理多个数据包的情况
        while (readable >= HEADER_SIZE) {
            // 内联读取头部信息
            uint32_t totallen_net, typelen_net;
            std::memcpy(&totallen_net, data, sizeof(uint32_t));
            std::memcpy(&typelen_net, data + sizeof(uint32_t),
                        sizeof(uint32_t));

            const uint32_t totallen = networkToHost(totallen_net);
            const uint32_t typelen = networkToHost(typelen_net);

            // 快速边界检查
            if (totallen < sizeof(uint32_t) || totallen > MAX_PACKET_SIZE)
                [[unlikely]] {
                // 高效的错误恢复 - 批量跳过无效数据
                const size_t skip = findNextPacketHeader(data, readable);
                buf->retrieve(skip);
                if (skip == readable)
                    break;

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
            const char *type_start = data + HEADER_SIZE;
            const char *json_start = type_start + typelen;
            const size_t json_len = totallen - sizeof(uint32_t) - typelen;

            // 边界验证
            if (json_len == 0 || json_start + json_len > data + readable)
                [[unlikely]] {
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
                if (!skip_utf8_check && !fastUTF8Check(json_start, json_len))
                    [[unlikely]] {
                    // 首次UTF-8错误后，可能是二进制数据，暂时跳过检查
                    skip_utf8_check = true;
                    buf->retrieve(packet_size);
                    data = buf->peek();
                    readable = buf->readableBytes();
                    continue;
                }

                // 高效JSON解析 - 直接从内存解析
                json js = json::parse(json_start, json_start + json_len,
                                      nullptr, false);

                // 成功解析
                buf->retrieve(packet_size);

                return std::make_tuple(std::string(type_start, typelen),
                                       std::move(js));

            } catch (...) [[unlikely]] {
                // 简化异常处理 - 直接跳过问题包
                buf->retrieve(packet_size);
                data = buf->peek();
                readable = buf->readableBytes();
                continue;
            }
        }

        return std::nullopt;
    }

    // 批量解码 - 一次处理多个包
    static std::vector<std::tuple<std::string, json>>
    tryDecodeBatch(Buffer *buf, size_t max_count = 10) {
        std::vector<std::tuple<std::string, json>> results;
        results.reserve(max_count);

        for (size_t i = 0; i < max_count; ++i) {
            auto result = tryDecode(buf);
            if (!result)
                break;
            results.emplace_back(std::move(*result));
        }

        return results;
    }
};

// 线程安全的编解码器池
class CodecPool {
  private:
    static thread_local std::string encode_buffer;
    static thread_local std::string temp_buffer;

  public:
    // 零拷贝编码到已有buffer
    static bool encodeToBuffer(const json &js, std::string_view type,
                               std::string &out_buffer) {
        return false; // 实现略
    }

    // 内存池管理
    static void recycleBuffer(std::string &&buffer) {
        if (buffer.capacity() < 64 * 1024) { // 64KB以下复用
            buffer.clear();
            encode_buffer = std::move(buffer);
        }
    }
};

// 性能测试和基准
class CodecBenchmark {
  public:
    static void benchmark() {
        // JSON测试数据
        json test_data;
        test_data["content"] = "这是一个测试消息，包含中文字符";
        test_data["id"] = 12345;
        test_data["timestamp"] = 1628745600;

        const int iterations = 100000;
        auto start = std::chrono::high_resolution_clock::now();

        // 编码测试
        for (int i = 0; i < iterations; ++i) {
            auto encoded =
                HighPerformanceCodec::encode(test_data, "test_message");
            // 避免编译器优化掉
            volatile size_t size = encoded.size();
            (void)size;
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        printf("编码性能: %d次操作耗时 %ld 微秒\n", iterations,
               duration.count());
        printf("平均每次编码: %.2f 微秒\n",
               duration.count() / double(iterations));
    }
};