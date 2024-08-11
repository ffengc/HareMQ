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
            if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || ch == '_' || ch == '.' || ch == '*' || ch == '#')
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
        if (type == ExchangeType::DIRECT) {
            return routing_key == binding_key;
        } else if (type == ExchangeType::FANOUT) {
            return true;
        } else {
            // dp 模式匹配
            return __dp_matching(routing_key, binding_key);
        }
    }

private:
    static bool __dp_matching(const std::string& routing_key, const std::string& binding_key) {
        // 1. 将binding_key与routing_key进行字符串分割，得到各个的单词数组
        std::vector<std::string> bkeys, rkeys;
        int n_bkey = string_helper::split(binding_key, ".", &bkeys);
        int n_rkey = string_helper::split(routing_key, ".", &rkeys);
        // 2. 定义标记数组，并初始化[0][0]位置为true，其他位置为false
        std::vector<std::vector<bool>> dp(n_bkey + 1, std::vector<bool>(n_rkey + 1, false));
        dp[0][0] = true;
        // 3. 如果binding_key以#起始，则将#对应行的第0位置置为1
        for (int i = 1; i <= bkeys.size(); ++i) {
            if (bkeys[i - 1] == "#") {
                dp[i][0] = true;
                continue;
            }
            break;
        }
        // 4. 使用routing_key中的每个单词与binding_key中的每个单词进行匹配并标记数组
        for (int i = 1; i <= n_bkey; i++) {
            for (int j = 1; j <= n_rkey; j++) {
                // 如果当前 bkey 是单词或者 * ，或者两个单词相同，表示单词匹配成功，从左上方继承结果
                if (bkeys[i - 1] == rkeys[j - 1] || bkeys[i - 1] == "*")
                    dp[i][j] = dp[i - 1][j - 1];
                // 如果当前 bkey 是 # , 则需要从左上，左边，上边继承结果
                else if (bkeys[i - 1] == "#")
                    dp[i][j] = dp[i - 1][j - 1] | dp[i][j - 1] | dp[i - 1][j];
            }
        }
        return dp[n_bkey][n_rkey];
    }
};
} // namespace hare_mq

#endif