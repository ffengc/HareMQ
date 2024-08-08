/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_VIRTUAL_HOST__
#define __YUFC_VIRTUAL_HOST__

#include "../mqcommon/logger.hpp"
#include "binding.hpp"
#include "exchange.hpp"
#include "message.hpp"
#include "queue.hpp"

namespace hare_mq {
class virtual_host {
public:
    using ptr = std::shared_ptr<virtual_host>; // ptr
private:
    std::string __host_name;
    exchange_manager::ptr __emp;
    msg_queue_manager::ptr __mqmp;
    binding_manager::ptr __bmp;
    message_manager::ptr __mmp; // 四个句柄
public:
    virtual_host(const std::string& host_name, const std::string& basedir, const std::string& dbfile)
        : __host_name(host_name)
        , __emp(std::make_shared<exchange_manager>(dbfile))
        , __mqmp(std::make_shared<msg_queue_manager>(dbfile))
        , __bmp(std::make_shared<binding_manager>(dbfile))
        , __mmp(std::make_shared<message_manager>(basedir)) {
        // 获取到所有的队列信息，通过队列名称恢复历史消息数据
        auto qm = __mqmp->all();
        for (auto& q : qm)
            __mmp->init_queue_msg(q.first); // 恢复了历史数据
    }
    bool declare_exchange(const std::string& name,
        ExchangeType type,
        bool durable,
        bool auto_delete,
        std::unordered_map<std::string, std::string>& args) {
        return __emp->declare_exchange(name, type, durable, auto_delete, args);
    } // 声明交换机
    void delete_exchange(const std::string& name) {
        // 删除交换机的时候，和这台交换机相关的绑定信息也要删除
        __bmp->unbind_exchange(name);
        __emp->delete_exchange(name);
    } // 删除交换机
    bool declare_queue(const std::string& qname,
        bool qdurable,
        bool qexclusive,
        bool qauto_delete,
        const std::unordered_map<std::string, std::string>& qargs) {
        // 初始化队列的消息句柄（消息的存储管理）
        // 队列的创建
        __mmp->init_queue_msg(qname);
        return __mqmp->declare_queue(qname, qdurable, qexclusive, qauto_delete, qargs);
    } // 声明队列
    void delete_queue(const std::string& name) {
        __mmp->destroy_queue_msg(name);
        __bmp->unbind_queue(name);
        __mqmp->delete_queue(name);
    } // 删除队列
    bool bind(const std::string& ename, const std::string& qname, const std::string& key) {
        exchange::ptr ep = __emp->select_exchange(ename);
        if (ep == nullptr) {
            LOG(ERROR) << "binding [exchange:" << ename << "] and [queue:" << qname << "] failed, exchange undefined" << std::endl;
            return false;
        }
        msg_queue::ptr mqp = __mqmp->select_queue(qname);
        if (mqp == nullptr) {
            LOG(ERROR) << "binding [exchange:" << ename << "] and [queue:" << qname << "] failed, queue undefined" << std::endl;
            return false;
        }
        return __bmp->bind(ename, qname, key, (ep->durable && mqp->durable)); // 需要两个都是持久化才能设置持久化
    } // 绑定交换机和队列
    void unbind(const std::string& ename, const std::string& qname) {
        __bmp->unbind(ename, qname);
        return;
    } // 解除绑定交换机和队列
    msg_queue_binding_map exchange_bindings(const std::string& ename) {
        return __bmp->get_exchange_bindings(ename); // 获取交换机的绑定信息
    } // 获取一台交换机的所有绑定信息
    bool basic_publish(const std::string& qname, BasicProperties* bp, const std::string& body) {
        msg_queue::ptr mqp = __mqmp->select_queue(qname); // 先找到这个队列
        if (mqp == nullptr) {
            LOG(ERROR) << "public failed, queue:" << qname << " undefined" << std::endl;
            return false;
        }
        return __mmp->insert(qname, bp, body, mqp->durable);
    } // 发布一条消息
    message_ptr basic_consume(const std::string& qname) {
        return __mmp->front(qname);
    } // 消费一条消息
    void basic_ack(const std::string& qname, const std::string& msgid) {
        __mmp->ack(qname, msgid);
    } // 确认一条消息
    void clear() {
        __emp->clear_exchange();
        __mqmp->clear_queues();
        __bmp->clear_bindings();
        __mmp->clear();
    }

public:
    // for debugs
    bool exists_exchange(const std::string& ename) { return __emp->exists(ename); }
    bool exists_queue(const std::string& qname) { return __mqmp->exists(qname); }
    bool exists_binding(const std::string& ename, const std::string& qname) { return __bmp->exists(ename, qname); }
};
} // namespace hare_mq

#endif