/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_QUEUE__
#define __YUFC_QUEUE__

#include "../mqcommon/helper.hpp"
#include "../mqcommon/logger.hpp"
#include "../mqcommon/msg.pb.h"
#include <assert.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace hare_mq {
/* 这一部分和exchange的实现基本一样 */
struct msg_queue {
    using ptr = std::shared_ptr<msg_queue>;
    std::string name; // 队列名称
    bool durable; // 持久化标志
    bool exclusive; // 是否独占
    bool auto_delete;
    std::unordered_map<std::string, std::string> args; // 其他参数
    msg_queue() { }
    msg_queue(const std::string& qname,
        bool qdurable,
        bool qexclusive,
        bool qauto_delete,
        const std::unordered_map<std::string, std::string>& qargs)
        : name(qname)
        , durable(qdurable)
        , exclusive(qexclusive)
        , auto_delete(qauto_delete)
        , args(qargs) { }
    // 下面两个接口和exchange的是完全一样的
    void set_args(const std::string& str_args) {
        std::vector<std::string> sub_args;
        size_t ret = string_helper::split(str_args, "&", &sub_args);
        for (auto& single_arg : sub_args) {
            size_t pos = single_arg.find("=");
            std::string key = single_arg.substr(0, pos);
            std::string val = single_arg.substr(pos + 1);
            args.insert({ key, val });
        }
    }
    std::string get_args() {
        std::string result;
        for (auto start = args.begin(); start != args.end(); ++start)
            result += start->first + "=" + start->second + "&";
        return result;
    }
};
using queue_map = std::unordered_map<std::string, msg_queue::ptr>;
#define QUEUE_CREATE_TABLE "create table if not exists queue_table(\
            name varchar(32) primary key, \
            durable int, \
            exclusive int, \
            auto_delete int, \
            args varchar(128));"
#define QUEUE_DROP_TABLE "drop table if exists queue_table;"
#define QUEUE_INSERT_SQL "insert into queue_table values('%s', %d, %d, %d, '%s');"
#define QUEUE_DELETE_SQL "delete from queue_table where name='%s';"
#define QUEUE_SELECT_SQL "select * from queue_table;"

class msg_queue_mapper {
private:
    sqlite_helper __sql_helper; // sql操作句柄
public:
    msg_queue_mapper(const std::string& dbfile)
        : __sql_helper(dbfile) {
        std::string path = file_helper::parent_dir(dbfile);
        file_helper::create_dir(path);
        assert(__sql_helper.open());
        create_table();
    }
    void create_table() {
        bool ret = __sql_helper.exec(QUEUE_CREATE_TABLE, nullptr, nullptr);
        if (ret == false)
            abort();
    }
    void remove_table() {
        bool ret = __sql_helper.exec(QUEUE_DROP_TABLE, nullptr, nullptr);
        if (ret == false)
            abort();
    }
    bool insert(msg_queue::ptr& q) {
        char sql_str[4096] = { 0 };
        sprintf(sql_str, QUEUE_INSERT_SQL,
            q->name.c_str(), q->durable == true ? 1 : 0,
            q->exclusive == true ? 1 : 0, q->auto_delete == true ? 1 : 0,
            q->get_args().c_str());
        return __sql_helper.exec(sql_str, nullptr, nullptr);
    }
    void remove(const std::string& name) {
        char sql_str[4096] = { 0 };
        sprintf(sql_str, QUEUE_DELETE_SQL, name.c_str());
        bool ret = __sql_helper.exec(sql_str, nullptr, nullptr);
    }
    queue_map all() {
        // recovery
        queue_map res;
        __sql_helper.exec(QUEUE_SELECT_SQL, select_callback, &res); // 这个是需要回调来组织查询回来的结果的
        return res;
    }

private:
    // typedef int (*sqlite_callback)(void*, int, char**, char**); // same to exchange
    static int select_callback(void* arg, int numcol, char** row, char** fields) {
        queue_map* result = (queue_map*)arg;
        auto qptr = std::make_shared<msg_queue>();
        qptr->name = row[0]; // row[0]是名字
        qptr->durable = (bool)std::stoi(row[1]);
        qptr->exclusive = (bool)std::stoi(row[2]);
        qptr->auto_delete = (bool)std::stoi(row[3]);
        if (row[4])
            qptr->set_args(row[4]);
        result->insert({ qptr->name, qptr });
        return 0;
    }
};

class msg_queue_manager {
public:
    using ptr = std::shared_ptr<msg_queue_manager>;
private:
    std::mutex __mtx;
    msg_queue_mapper __mapper;
    queue_map __msg_queues; // all same to exchange
public:
    msg_queue_manager(const std::string& dbfile)
        : __mapper(dbfile) {
        __msg_queues = __mapper.all(); // recovery
    }
    bool declare_queue(const std::string& qname,
        bool qdurable,
        bool qexclusive,
        bool qauto_delete,
        const std::unordered_map<std::string, std::string>& qargs) {
        // all same to exchange
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __msg_queues.find(qname);
        if (it != __msg_queues.end())
            return true;
        auto qptr = std::make_shared<msg_queue>(qname, qdurable, qexclusive, qauto_delete, qargs);
        if (qdurable == true) {
            if (!__mapper.insert(qptr))
                return false;
        }
        __msg_queues.insert({ qname, qptr });
        return true;
    }
    void delete_queue(const std::string& name) {
        // all same to exchange
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __msg_queues.find(name);
        if (it == __msg_queues.end())
            return;
        if (it->second->durable == true)
            __mapper.remove(name);
        __msg_queues.erase(name);
    }
    msg_queue::ptr select_queue(const std::string& name) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __msg_queues.find(name);
        if (it == __msg_queues.end())
            return msg_queue::ptr();
        return it->second;
    }
    queue_map all() {
        std::unique_lock<std::mutex> lock(__mtx);
        return __msg_queues;
    }
    bool exists(const std::string& name) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __msg_queues.find(name);
        return it == __msg_queues.end() ? false : true;
    }
    size_t size() {
        std::unique_lock<std::mutex> lock(__mtx);
        return __msg_queues.size();
    }
    void clear_queues() {
        std::unique_lock<std::mutex> lock(__mtx);
        __mapper.remove_table();
        __msg_queues.clear();
    }
};
} // namespace hare_mq

#endif