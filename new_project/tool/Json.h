#pragma once

#include "../muduo/logging/Logging.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 提取json变量
template <typename T>
std::optional<T> ExtractCommonField(const json &j, const std::string &key) {
    try {
        if (j.contains(key) && !j.at(key).is_null()) {
            return j.at(key).get<T>();
        }
    } catch (const std::exception &e) {
        LOG_ERROR << "Extract field [" << key << "] error: " << e.what();
        // 也可以记录当前的json值
        LOG_ERROR << "Json value for key [" << key
                  << "] is: " << j.at(key).dump();
    }
    return std::nullopt;
}

// 更完备的提取json变量
template <typename T>
bool AssignIfPresent(const json &j, const std::string &key, T &out) {
    if (j.is_string()) {
        try {
            std::string content = j.get<std::string>();
            if (content.empty()) {
                LOG_ERROR << "尝试解析空字符串为 JSON,key: " << key;
                return false;
            }

            json pared = json::parse(content);
            return AssignIfPresent(pared, key, out);
        } catch (...) {
            return false;
        }
    }

    auto opt = ExtractCommonField<T>(j, key);
    if (opt.has_value()) {
        out = std::move(opt.value());
        return true;
    }
    return false;
}