/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqcommon/helper.hpp"
#include "../mqcommon/logger.hpp"
#include "../mqcommon/msg.pb.h"
#include <assert.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace hare_mq {
/**
 * 1. 交换机类
 * 2. 交换机数据持久化管理类
 * 3. 交换机数据内存管理类
 */
struct exchange {
    /* 交换机类 */
public:
    using ptr = std::shared_ptr<exchange>;
    std::string name; // 交换机名称
    ExchangeType type; // 交换机类型
    bool durable; // 持久化标志
    bool auto_delete; // 自动删除标志
    std::unordered_map<std::string, std::string> args; // 其他参数
public:
    exchange(const std::string ename,
        ExchangeType etype,
        bool edurable,
        bool eauto_delete,
        const std::unordered_map<std::string, std::string>& eargs)
        : name(ename)
        , type(etype)
        , auto_delete(eauto_delete)
        , args(eargs) { }
    // args存储的格式是键值对，在存储数据库的时候，会组织一个字符串进行存储 key=value&key=value
    void set_args(const std::string& str_args) {
        /**
         * 解析 str_args 字符串: key=value&key=value... 存到 args 成员变量中去
         */
        // 用字符串切割就行了
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
        /**
         * set_args()的反操作，把args里面的数据序列化成 key=value&key=value... 的格式
         */
        std::string result;
        for (auto start = args.begin(); start != args.end(); ++start)
            result += start->first + "=" + start->second + "&";
        return result;
    }
};

// 创建表的sql语句
#define CREATE_TABLE "create table if not exists exchange_table(\
            name varchar(32) primary key, \
            type int, \
            durable int, \
            delete int, \
            args varchar(128));"
// 删除表的sql语句
#define DROP_TABLE "drop table if not exists exchange_table;"
// 新增交换机的sql语句
#define INSERT_SQL "insert into exchange_table values('%s', %d, %d, %d, '%s');"
// 删除交换机的sql语句
#define DELETE_SQL "delete from exchange_table where name='%s';"
// 查询获取所有交换机的sql语句
#define SELECT_SQL "select name, type, durable, auto_delete, args from exchange_table;"

class exchange_mapper {
    /* 交换机数据持久化管理类 */
private:
    sqlite_helper __sql_helper; // sqlite操作句柄
public:
    exchange_mapper(const std::string& dbfile)
        : __sql_helper(dbfile) {
        // 构造，需要传递数据库文件名称
        std::string path = file_helper::parent_dir(dbfile);
        file_helper::create_dir(path);
        assert(__sql_helper.open());
    }

public:
    void create_table() {
        // 创建表
        bool ret = __sql_helper.exec(CREATE_TABLE, nullptr, nullptr);
        if (ret == false)
            abort();
    }
    void remove_table() {
        // 删除表
        bool ret = __sql_helper.exec(DROP_TABLE, nullptr, nullptr);
        if (ret == false)
            abort();
    }
    void insert(exchange::ptr& e) {
        // 插入交换机
        char sql_str[4096] = { 0 };
        sprintf(sql_str, INSERT_SQL,
            e->name, e->type,
            e->durable, e->auto_delete,
            e->get_args().c_str());
        bool ret = __sql_helper.exec(sql_str, nullptr, nullptr);
    }
    void remove(const std::string& name) {
        // 移除交换机
        char sql_str[4096] = { 0 };
        sprintf(sql_str, DELETE_SQL, name.c_str());
        bool ret = __sql_helper.exec(sql_str, nullptr, nullptr);
    }
    std::unordered_map<std::string, exchange::ptr> all() {
        // recovery
        std::unordered_map<std::string, exchange::ptr> res;
        __sql_helper.exec(SELECT_SQL, select_callback, &res); // 这个是需要回调来组织查询回来的结果的
        return res;
    }

private:
    // typedef int (*sqlite_callback)(void*, int, char**, char**);
    static int select_callback(void* arg, int numcol, char** row, char** fields) {
        // 需要static, 避免this
        std::unordered_map<std::string, exchange::ptr>* result
            = (std::unordered_map<std::string, exchange::ptr>*)arg;
        auto exp = std::make_shared<exchange>();
        exp->name = row[0]; // row[0]是名字
        exp->type = (ExchangeType)std::stoi(row[1]); // row[1]是type, 都是按顺序的 row 就是一个C风格的字符串数组
        exp->durable = (bool)std::stoi(row[2]);
        exp->auto_delete = (bool)std::stoi(row[3]);
        if (row[4])
            exp->set_args(row[4]); // 注意这个字段是不一定有的, 如果null就不要设置
        result->insert({ exp->name, exp });
        return 0; // return 0; 必须有, 不然sqlite会出问题，这个demo里面也提到了
    }
};

class exchange_manager {
    /* 交换机数据内存管理类 */
private:
    exchange_mapper __mapper; // 持久化管理
    std::unordered_map<std::string, exchange::ptr> __exchanges; // 管理所有的交换机
    std::mutex __mtx; // exchange_manager 会被多线程调用，管理一个互斥锁
public:
    exchange_manager(const std::string& dbfile);
    void declare_exchange(const std::string& name,
        ExchangeType type,
        bool durable,
        bool auto_delete,
        std::unordered_map<std::string, std::string>& args); // 声明交换机
    void delete_exchange(const std::string& name); // 删除交换机
    exchange::ptr select_exchange(const std::string& name); // 选择一台交换机
    bool exists(const std::string& name); // 判断交换机是否存在
    void clear_exchange(); // 清理所有交换机
};

} // namespace hare_mq
