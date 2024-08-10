/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_ROUTE__
#define __YUFC_ROUTE__

#include "../mqcommon/helper.hpp"
#include "../mqcommon/logger.hpp"
#include "../mqcommon/msg.pb.h"

namespace hare_mq {
class router {
public:
    static bool is_legal_routing_key(const std::string& routing_key) {
        // 只需要判断是否包含非法字符即可
        for (const auto& ch : routing_key) {
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '.')
                continue;
            return false;
        }
        return true;
    }
    static bool is_legal_binding_key(const std::string& binding_key) {
        // 1. 判断是否包含非法字符
        for (const auto& ch : binding_key) {
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' && ch == '.' || ch == '*' || ch == '#')
                continue;
            return false;
        }
        // 2. * # 必须独立存在
        std::vector<std::string> sub;
        string_helper::split(binding_key, ".", &sub);
        for (const auto& e : sub) {
            if (e.size() > 1 && (e.find('*') != std::string::npos || e.find('#') != std::string::npos))
                return false;
        }
        // 3. * # 不能连续出现
        for (size_t i = 1; i < sub.size(); ++i) {
            if ((sub[i] == "#" && sub[i - 1] == "*") || (sub[i] == "#" && sub[i - 1] == "#") || (sub[i] == "*" && sub[i - 1] == "#"))
                return false;
        }
        return true;
    }
    static bool route(ExchangeType type, const std::string& routing_key, const std::string& binding_key) {
        if(type == ExchangeType::DIRECT) {
            return routing_key == binding_key;
        } else if(type == ExchangeType::FANOUT) {
            return true;
        } else {
            // dp 模式匹配
        }
    }
};
} // namespace hare_mq

#endif