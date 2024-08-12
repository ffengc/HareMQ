/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_CONSUMER__
#define __YUFC_CONSUMER__

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
    consumer(const std::string& ctag, const std::string& queue_name, bool ack_flag, const consumer_callback& cb)
        : tag(ctag)
        , qname(queue_name)
        , auto_ack(ack_flag)
        , callback(cb) { }
};

/* 以队列为单元的消费者结构 */
class queue_consumer {
private:
    std::string __qname;
    std::mutex __mtx;
    uint64_t __rr_seq; // 轮转序号
    std::vector<consumer::ptr> __consumers; // 管理的所有消费者对象
public:
    using ptr = std::shared_ptr<queue_consumer>;
    queue_consumer(const std::string& qname)
        : __qname(qname)
        , __rr_seq(0) { }
    consumer::ptr create(const std::string& ctag, const std::string& queue_name, bool ack_flag, const consumer_callback& cb) {
        // 1. lock
        std::unique_lock<std::mutex> lock(__mtx);
        // 2. 判断消费者是否重复
        for (const auto& e : __consumers)
            if (e->tag == ctag) {
                LOG(WARNING) << "consumer duplicate tag, create consumer failed" << std::endl;
                return consumer::ptr(); // 创建失败
            }
        // 3. 没有重复则新增，构造对象
        auto new_consumer = std::make_shared<consumer>(ctag, queue_name, ack_flag, cb);
        // 4. 田间管理后返回对象
        __consumers.push_back(new_consumer);
        return new_consumer;
    } // 创建消费者
    void remove(const std::string& ctag) {
        std::unique_lock<std::mutex> lock(__mtx);
        for (auto it = __consumers.begin(); it != __consumers.end(); ++it) {
            if ((*it)->tag == ctag) {
                __consumers.erase(it);
                return;
            }
        }
        LOG(WARNING) << "consumer not founded, remove failed" << std::endl;
        return;
    } // 删除一个消费者
    consumer::ptr rr_choose() {
        std::unique_lock<std::mutex> lock(__mtx);
        if (__consumers.size() == 0)
            return consumer::ptr();
        int idx = __rr_seq % __consumers.size();
        ++__rr_seq;
        return __consumers[idx];
    } // rr 轮转获取一个消费者
    bool empty() {
        std::unique_lock<std::mutex> lock(__mtx);
        return __consumers.size() == 0;
    } // 判空
    bool exists(const std::string& ctag) {
        std::unique_lock<std::mutex> lock(__mtx);
        for (auto it = __consumers.begin(); it != __consumers.end(); ++it)
            if ((*it)->tag == ctag)
                return true;
        return false;
    } // 判断消费者是否存在
    void clear() {
        std::unique_lock<std::mutex> lock(__mtx);
        __consumers.clear();
        __rr_seq = 0;
    } // 清理所有消费者
};
/* 对外提供的消费者管理 */
class consumer_manager {
private:
    std::mutex __mtx;
    std::unordered_map<std::string, queue_consumer::ptr> __queue_consumers; // 映射关系
public:
    using ptr = std::shared_ptr<consumer_manager>;
    consumer_manager() = default;
    void init_queue_consumer(const std::string& qname) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __queue_consumers.find(qname);
        if (it != __queue_consumers.end())
            return;
        auto new_qconsumer = std::make_shared<queue_consumer>(qname);
        __queue_consumers.insert({ qname, new_qconsumer });
    }
    void destroy_queue_consumer(const std::string& qname) {
        std::unique_lock<std::mutex> lock(__mtx);
        __queue_consumers.erase(qname);
    }
    consumer::ptr create(const std::string& ctag,
        const std::string& queue_name,
        bool ack_flag,
        const consumer_callback& cb) {
        queue_consumer::ptr qcp;
        {
            std::unique_lock<std::mutex> lock(__mtx); // 这个锁是保护查找操作的
            auto it = __queue_consumers.find(queue_name);
            if (it == __queue_consumers.end()) {
                LOG(ERROR) << "cannot find this queue_consumer handler: [" << queue_name << "]" << std::endl;
                return consumer::ptr();
            }
            qcp = it->second;
        }
        return qcp->create(ctag, queue_name, ack_flag, cb); // 这里面有自己的锁
    }
    void remove(const std::string& ctag, const std::string& queue_name) {
        queue_consumer::ptr qcp;
        {
            std::unique_lock<std::mutex> lock(__mtx); // 这个锁是保护查找操作的
            auto it = __queue_consumers.find(queue_name);
            if (it == __queue_consumers.end()) {
                LOG(ERROR) << "cannot find this queue_consumer handler: [" << queue_name << "]" << std::endl;
                return;
            }
            qcp = it->second;
        }
        return qcp->remove(ctag);
    } // 删除指定队列里面的指定消费者
    consumer::ptr choose(const std::string& queue_name) {
        queue_consumer::ptr qcp;
        {
            std::unique_lock<std::mutex> lock(__mtx); // 这个锁是保护查找操作的
            auto it = __queue_consumers.find(queue_name);
            if (it == __queue_consumers.end()) {
                LOG(ERROR) << "cannot find this queue_consumer handler: [" << queue_name << "]" << std::endl;
                return consumer::ptr();
            }
            qcp = it->second;
        }
        return qcp->rr_choose();
    }
    bool empty(const std::string& queue_name) {
        queue_consumer::ptr qcp;
        {
            std::unique_lock<std::mutex> lock(__mtx); // 这个锁是保护查找操作的
            auto it = __queue_consumers.find(queue_name);
            if (it == __queue_consumers.end()) {
                LOG(ERROR) << "cannot find this queue_consumer handler: [" << queue_name << "]" << std::endl;
                return false;
            }
            qcp = it->second;
        }
        return qcp->empty();
    }
    bool exists(const std::string& ctag, const std::string& queue_name) {
        queue_consumer::ptr qcp;
        {
            std::unique_lock<std::mutex> lock(__mtx); // 这个锁是保护查找操作的
            auto it = __queue_consumers.find(queue_name);
            if (it == __queue_consumers.end()) {
                LOG(ERROR) << "cannot find this queue_consumer handler: [" << queue_name << "]" << std::endl;
                return false;
            }
            qcp = it->second;
        }
        return qcp->exists(ctag);
    }
    void clear() {
        std::unique_lock<std::mutex> lock(__mtx);
        __queue_consumers.clear();
    }
};
} // namespace hare_mq

#endif