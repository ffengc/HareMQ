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
#include <errno.h>
#include <fstream>
#include <google/protobuf/map.h>
#include <iomanip>
#include <iostream>
#include <random>
#include <sqlite3.h>
#include <sstream>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <unordered_map>
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
    static std::string uuid() {
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

class file_helper {
private:
    std::string __file_name;

public:
    std::string path() { return __file_name; }

public:
    file_helper(const std::string& file_name)
        : __file_name(file_name) { }
    ~file_helper() = default;

public:
    bool exists() {
        struct stat st;
        return stat(__file_name.c_str(), &st) == 0;
    }
    size_t size() {
        struct stat st;
        int ret = stat(__file_name.c_str(), &st);
        if (ret < 0)
            return 0; // 文件不存在
        return st.st_size; // 这个是文件的大小
    }
    bool read(std::string& body) {
        // 获取文件大小，根据文件大小调整body的空间
        size_t fsize = this->size();
        // 调整空间
        body.resize(fsize);
        return read(&body[0], 0, fsize);
    }
    bool read(char* body, size_t offset, size_t len) {
        // 1. 打开文件
        std::ifstream ifs(__file_name, std::ios::binary | std::ios::in);
        if (ifs.is_open() == false) {
            LOG(ERROR) << "file open error: " << __file_name << std::endl;
            return false;
        }
        // 2. 跳转读写位置
        ifs.seekg(offset, std::ios::beg);
        // 3. 读取文件数据
        ifs.read(body, len);
        if (ifs.good() == false) {
            LOG(ERROR) << "read data error: " << __file_name << std::endl;
            ifs.close();
            return false;
        }
        // 4. 关闭文件
        ifs.close();
        return true;
    }
    bool write(const std::string& body) {
        return write(body.c_str(), 0, body.size());
    }
    bool write(const char* body, size_t offset, size_t len) {
        // 1. 打开文件
        std::fstream fs(__file_name, std::ios::binary | std::ios::out | std::ios::in);
        if (fs.is_open() == false) {
            LOG(ERROR) << "file open error: " << __file_name << std::endl;
            return false;
        }
        // 2. 跳转到文件指定位置
        fs.seekp(offset, std::ios::beg);
        // 3. 写入数据
        fs.write(body, len);
        if (fs.good() == false) {
            LOG(ERROR) << "write data error: " << __file_name << std::endl;
            fs.close();
            return false;
        }
        // 4. 关闭文件
        fs.close();
        return true;
    }
    static bool create(const std::string& file_name) {
        std::ofstream ofs(file_name, std::ios::binary | std::ios::out);
        if (ofs.is_open() == false) {
            LOG(ERROR) << "create file error: " << file_name << "  # " << strerror(errno) << std::endl;
            return false;
        }
        ofs.close();
        return true;
    }
    static bool remove(const std::string& file_name) {
        return ::remove(file_name.c_str()) == 0;
    }
    static bool create_dir(const std::string& path) {
        // aaa/bbb/ccc/ddd
        // 在多级路径创建中，需要从第一个父级目录开始创建
        // 第一个创建的是 aaa
        // 第二个是 aaa/bbb 而不是 bbb
        // ...
        size_t pos, idx = 0;
        while (idx < path.size()) {
            pos = path.find("/", idx);
            if (pos == std::string::npos) {
                return ::mkdir(path.c_str(), 0775) == 0;
            }
            std::string subpath = path.substr(0, pos);
            int ret = ::mkdir(subpath.c_str(), 0775);
            if (ret != 0 && errno != EEXIST) {
                LOG(ERROR) << "create dir: " << path << " failed" << std::endl;
                return false;
            }
            idx = pos + 1;
        }
        return true;
    }
    static bool remove_dir(const std::string& path) {
        // rm -rf
        // system() 可以运行命令
        std::string cmd = "rm -rf " + path;
        return (system(cmd.c_str()) != -1);
    }
    bool rename(const std::string& name) {
        // stdio.h 里面有库函数
        return (::rename(__file_name.c_str(), name.c_str()) == 0); // 一定要加::
    }
    static std::string parent_dir(const std::string& file_name) {
        // aaa/bbb/ccc/test.txt
        // 获取父级目录
        size_t pos = file_name.find_last_of("/");
        if (pos == std::string::npos) // 当前目录
            return "./";
        std::string path = file_name.substr(0, pos);
        return path;
    }
};

class map_helper {
public:
    static std::unordered_map<std::string, std::string> ConvertProtoMapToStdMap(const google::protobuf::Map<std::string, std::string>& proto_map) {
        std::unordered_map<std::string, std::string> std_map;
        for (const auto& kv : proto_map) {
            std_map[kv.first] = kv.second;
        }
        return std_map;
    }
    static google::protobuf::Map<std::string, std::string> ConvertStdMapToProtoMap(const std::unordered_map<std::string, std::string>& std_map) {
        google::protobuf::Map<std::string, std::string> proto_map;
        for (const auto& kv : std_map) {
            proto_map[kv.first] = kv.second;
        }
        return proto_map;
    }
};
} // namespace hare_mq

#endif