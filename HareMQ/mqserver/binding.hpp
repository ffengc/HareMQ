/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_BINDING__
#define __YUFC_BINDING__

#include "../mqcommon/helper.hpp"
#include "../mqcommon/logger.hpp"
#include "../mqcommon/msg.pb.h"
#include <assert.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace hare_mq {
/* 绑定信息类 */
struct binding {
    using ptr = std::shared_ptr<binding>;
    std::string exchange_name;
    std::string msg_queue_name;
    std::string binding_key;
    binding() { }
    binding(const std::string& ename, const std::string& qname, const std::string& key)
        : exchange_name(ename)
        , msg_queue_name(qname)
        , binding_key(key) { }
};

#define BINDING_CREATE_TABLE "create table if not exists binding_table(\
            exchange_name varchar(32), \
            msg_queue_name varchar(32), \
            binding_key varchar(128));"
#define BINDING_DROP_TABLE "drop table if exists binding_table;"
#define BINDING_INSERT_SQL "insert into binding_table values('%s', '%s', '%s');"
#define BINDING_DELETE_SQL "delete from binding_table where exchange_name='%s' and msg_queue_name='%s';"
#define BINDING_DELETE_EXCHANGE_SQL "delete from binding_table where exchange_name='%s';"
#define BINDING_DELETE_QUEUE_SQL "delete from binding_table where msg_queue_name='%s';"
#define BINDING_SELECT_SQL "select * from binding_table;"
using msg_queue_binding_map = std::unordered_map<std::string, binding::ptr>; // tips in doc
using binding_map = std::unordered_map<std::string, msg_queue_binding_map>;
/* 绑定信息数据持久化类 */
class binding_mapper {
private:
    sqlite_helper __sql_helper; // sql操作句柄
public:
    binding_mapper(const std::string& dbfile)
        : __sql_helper(dbfile) {
        std::string path = file_helper::parent_dir(dbfile);
        file_helper::create_dir(path);
        assert(__sql_helper.open());
        create_table();
    } // constructor
public:
    void create_table() {
        // same to queue and exchange
        bool ret = __sql_helper.exec(BINDING_CREATE_TABLE, nullptr, nullptr);
        if (ret == false)
            abort();
    }
    void remove_table() {
        // same to queue and exchange
        bool ret = __sql_helper.exec(BINDING_DROP_TABLE, nullptr, nullptr);
        if (ret == false)
            abort();
    }
    bool insert(binding::ptr& obj) {
        // same to queue and exchange
        char sql_str[4096] = { 0 };
        sprintf(sql_str, BINDING_INSERT_SQL,
            obj->exchange_name.c_str(),
            obj->msg_queue_name.c_str(),
            obj->binding_key.c_str());
        return __sql_helper.exec(sql_str, nullptr, nullptr);
    }
    void remove(const std::string& ename, const std::string& qname) {
        char sql_str[4096] = { 0 };
        sprintf(sql_str, BINDING_DELETE_SQL, ename.c_str(), qname.c_str());
        bool ret = __sql_helper.exec(sql_str, nullptr, nullptr);
    } // 移除特定的联系
    void remove_exchange(const std::string& ename) {
        char sql_str[4096] = { 0 };
        sprintf(sql_str, BINDING_DELETE_EXCHANGE_SQL, ename.c_str());
        bool ret = __sql_helper.exec(sql_str, nullptr, nullptr);
    } // 移除特定交换机的联系
    void remove_queue(const std::string& qname) {
        char sql_str[4096] = { 0 };
        sprintf(sql_str, BINDING_DELETE_QUEUE_SQL, qname.c_str());
        bool ret = __sql_helper.exec(sql_str, nullptr, nullptr);
    } // 移除特定队列的联系
    binding_map all() {
        binding_map res;
        __sql_helper.exec(BINDING_SELECT_SQL, select_callback, &res); // 这个是需要回调来组织查询回来的结果的
        return res;
    } // recovery
private:
    static int select_callback(void* arg, int numcol, char** row, char** fields) {
        binding_map* result = (binding_map*)arg;
        binding::ptr bp = std::make_shared<binding>(row[0], row[1], row[2]);
        msg_queue_binding_map& qmap = (*result)[bp->exchange_name]; // 这里比较巧妙
        qmap.insert({ bp->msg_queue_name, bp });
        return 0;
    }
};

/* 绑定信息数据管理类 */
class binding_manager {
private:
    std::mutex __mtx;
    binding_mapper __mapper;
    binding_map __bindings; // all same as queue.hpp and exchange.hpp
public:
    using ptr = std::shared_ptr<binding_manager>;
    binding_manager(const std::string& dbfile)
        : __mapper(dbfile) {
        __bindings = __mapper.all(); // recovery
    } // contructor
public:
    bool bind(const std::string& ename, const std::string& qname, const std::string& key, bool durable) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __bindings.find(ename);
        if (it != __bindings.end() && it->second.find(qname) != it->second.end()) // 这样才表示绑定信息存在
            return true;
        // 绑定信息是否需要持久化取决于: 交换机持久化 && 队列也是持久化的
        binding::ptr bp = std::make_shared<binding>(ename, qname, key);
        if (durable) {
            bool ret = __mapper.insert(bp);
            if (ret == false)
                return false;
        }
        auto& qbmap = __bindings[ename]; // 先获取，不存在则会创建
        qbmap.insert({ qname, bp });
        return true;
    } // add a bind
    void unbind(const std::string& ename, const std::string& qname) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __bindings.find(ename);
        if (it == __bindings.end()) // 交换机的数据都无
            return;
        if (it->second.find(qname) == it->second.end()) // 有交换机，但是没有绑定到qname上，也是无
            return;
        __mapper.remove(ename, qname);
        __bindings[ename].erase(qname);
    } // remove a bind
    void unbind_exchange(const std::string& ename) {
        std::unique_lock<std::mutex> lock(__mtx);
        __mapper.remove_exchange(ename);
        __bindings.erase(ename);
    } // remove a exchange's binds
    void unbind_queue(const std::string& qname) {
        std::unique_lock<std::mutex> lock(__mtx);
        __mapper.remove_queue(qname);
        // 一个queue可能和多个exchange都有绑定信息，如何都删除掉? 所以要遍历
        for (auto start = __bindings.begin(); start != __bindings.end(); ++start)
            start->second.erase(qname);
    } // remove a queue's binds
    msg_queue_binding_map get_exchange_bindings(const std::string& ename) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __bindings.find(ename);
        if (it == __bindings.end()) // 交换机的数据无
            return msg_queue_binding_map(); // return null;
        return it->second;
    } // get exchange binding info
    binding::ptr get_binding(const std::string& ename, const std::string& qname) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __bindings.find(ename);
        if (it == __bindings.end()) // 交换机的数据无
            return binding::ptr(); // return null;
        auto qit = it->second.find(qname);
        if (qit == it->second.end())
            return binding::ptr();
        return qit->second;
    } // get one binding info
    bool exists(const std::string& ename, const std::string& qname) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __bindings.find(ename);
        if (it == __bindings.end())
            return false;
        auto qit = it->second.find(qname);
        if (qit == it->second.end())
            return false;
        return true;
    }
    size_t size() {
        size_t total_size = 0;
        std::unique_lock<std::mutex> lock(__mtx);
        for (auto start = __bindings.begin(); start != __bindings.end(); ++start)
            total_size += start->second.size(); // 遍历交换机，看看每个交换机有多少个，都要加起来
        return total_size;
    }
    void clear_bindings() {
        std::unique_lock<std::mutex> lock(__mtx);
        __mapper.remove_table();
        __bindings.clear();
    }
};

} // namespace hare_mq

#endif