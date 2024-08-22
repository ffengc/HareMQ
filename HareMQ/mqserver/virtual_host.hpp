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
#include <tuple>

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
    queue_map all_queues() {
        return __mqmp->all();
    }
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
    std::string basic_query() {
        std::string yellow_bold = "\033[1;33m"; // 1 表示加粗, 33 表示前景色为黄色
        std::string reset = "\033[0m"; // 重置样式
        std::vector<std::tuple<std::string, std::string, std::string>> all_bindings;
        // 获取所有交换机
        auto all_ex = __emp->select_all_exchanges();
        std::string res_str;
        res_str += yellow_bold + "------------------------ Query ------------------------\n" + reset;
        res_str += yellow_bold + "exists exchanges: " + reset + '\n';
        res_str += "    ";
        for (const auto& e : all_ex) {
            // 先获取这台交换机的所有绑定信息
            msg_queue_binding_map this_ex_bindings = exchange_bindings(e.first);
            for (const auto& q : this_ex_bindings) {
                assert(q.second->exchange_name == e.first);
                all_bindings.push_back(std::make_tuple(q.second->exchange_name, q.second->msg_queue_name, q.second->binding_key));
            }
            res_str += e.first;
            res_str += ", ";
        }
        res_str += '\n';
        // 获取所有队列
        auto all_q = all_queues();
        res_str += yellow_bold + "exists queues: \n" + reset;
        res_str += "    ";
        for (const auto& e : all_q) {
            res_str += e.first;
            res_str += ", ";
        }
        res_str += '\n';
        // 整理所有binding
        res_str += yellow_bold + "exists bindings: \n" + reset;
        for (const auto& e : all_bindings) {
            std::string tmp_ename = std::get<0>(e);
            std::string tmp_qname = std::get<1>(e);
            std::string tmp_binding_key = std::get<2>(e);
            std::string line = "    " + tmp_ename + "<-->" + tmp_qname + ", binding_key: " + tmp_binding_key + "\n";
            res_str += line;
        }
        res_str += yellow_bold + "-------------------------------------------------------" + reset;
        return res_str;
    }
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
    exchange::ptr select_exchange(const std::string& ename) { return __emp->select_exchange(ename); }
};
} // namespace hare_mq

#endif