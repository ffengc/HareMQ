/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_COMMON_HELPER__
#define __YUFC_COMMON_HELPER__

#include "./logger.hpp"
#include <atomic>
#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <iostream>
#include <random>
#include <sqlite3.h>
#include <sstream>
#include <string>
#include <vector>

namespace hare_mq {
class sqlite_helper {
public:
    typedef int (*sqlite_callback)(void*, int, char**, char**);

private:
    sqlite3* __handler;
    std::string __db_file;

public:
    sqlite_helper(const std::string& db_file)
        : __db_file(db_file)
        , __handler(nullptr) { }
    bool open(int safe_lavel = SQLITE_OPEN_FULLMUTEX) {
        // 打开数据库(文件)
        // int sqlite3_open_v2(const char* filename, sqlite3 **ppDb, int flags, const char* zVfs);
        int ret = sqlite3_open_v2(__db_file.c_str(), &__handler, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | safe_lavel, nullptr);
        if (ret != SQLITE_OK) {
            LOG(ERROR) << "create database failed: " << sqlite3_errmsg(__handler) << std::endl;
            return false;
        }
        return true;
    }
    bool exec(const std::string& sql, sqlite_callback cb, void* arg) {
        // 执行语句
        // int sqlite3_exec(sqlite3*, char* sql, int (*callback)(void*, int, char**, char**), void* arg, char**err);
        int ret = sqlite3_exec(__handler, sql.c_str(), cb, arg, nullptr);
        if (ret != SQLITE_OK) {
            LOG(ERROR) << "run exec: [" << sql << "] failed: " << sqlite3_errmsg(__handler) << std::endl;
            return false;
        }
        return true;
    }
    bool close() {
        // 关闭数据库(文件)
        if (__handler) {
            if (sqlite3_close_v2(__handler) == SQLITE_OK)
                return true;
            LOG(ERROR) << "close error" << std::endl;
            return false;
        }
        LOG(ERROR) << "null sql handler" << std::endl;
        return false;
    }
};

class string_helper {
public:
    static size_t split(const std::string& str, const std::string& sep, std::vector<std::string>* out, bool if_compress = true) {
        // boost split
        if (if_compress) {
            boost::split(*out, str, boost::is_any_of(sep), boost::token_compress_on);
            return out->size();
        } else {
            boost::split(*out, str, boost::is_any_of(sep), boost::token_compress_off);
            return out->size();
        }
    }
};

class uuid_helper {
public:
    std::string uuid() {
        std::random_device rd;
        std::mt19937_64 generator(rd());
        std::uniform_int_distribution<int> distribution(0, 255);
        std::stringstream ss;
        for (int i = 0; i < 8; ++i) {
            ss << std::setw(2) << std::setfill('0') << std::hex << distribution(generator);
            if (i == 3 || i == 5 || i == 7)
                ss << "-";
            static std::atomic<size_t> seq(1); // 这里一定要静态，保证多次调用都是自增的
            size_t num = seq.fetch_add(1);
            for (int i = 7; i >= 0; i--) {
                ss << std::setw(2) << std::setfill('0') << std::hex << ((num >> (i * 8)) & 0xff);
                if (i == 6)
                    ss << "-";
            }
        }
        return ss.str();
    }
};
} // namespace hare_mq

#endif