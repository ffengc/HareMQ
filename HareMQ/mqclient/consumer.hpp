/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_CLIENT_CONSUMER__
#define __YUFC_CLIENT_CONSUMER__

#include "../mqcommon/helper.hpp"
#include "../mqcommon/logger.hpp"
#include "../mqcommon/msg.pb.h"
#include <assert.h>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace hare_mq {
using consumer_callback = std::function<void(const std::string&, const BasicProperties*, const std::string&)>;
struct consumer {
    using ptr = std::shared_ptr<consumer>;
    std::string tag; // 消费者标识
    std::string qname; // 订阅的队列名称
    bool auto_ack; // 自动确认标志
    consumer_callback callback; // 回调
    consumer() { }
    ~consumer() = default;
    consumer(const std::string& ctag, const std::string& queue_name, bool ack_flag, const consumer_callback& cb)
        : tag(ctag)
        , qname(queue_name)
        , auto_ack(ack_flag)
        , callback(cb) { }
};
} // namespace hare_mq
#endif