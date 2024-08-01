/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_MESSAGE__
#define __YUFC_MESSAGE__

#include "../mqcommon/helper.hpp"
#include "../mqcommon/logger.hpp"
#include "../mqcommon/msg.pb.h"
#include <assert.h>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace hare_mq {
#define DATAFILE_SUBFIX ".mqd"
#define TMPFILE_SUBFIX ".mqd.tmp" // 定义持久化文件和临时文件的文件名后缀
using message_ptr = std::shared_ptr<Message>;
class message_mapper {
private:
    std::string __queue_name; // 队列名
    std::string __data_file; // 持久化文件
    std::string __tmp_file; // 临时文件
public:
    message_mapper(const std::string& base_dir, const std::string& qname)
        : __queue_name(qname) {
        std::string dir = base_dir; // deep copy
        if (dir.back() != '/')
            dir.push_back('/');
        __data_file = dir + qname + DATAFILE_SUBFIX;
        __tmp_file = dir + qname + TMPFILE_SUBFIX;
        if (!file_helper::create_dir(dir)) {
            // 先把这个目录创建出来
            LOG(FATAL) << "message_mapper()->create_dir(): " << dir << " failed" << std::endl;
            abort();
        }
    }
    bool create_msg_file() {
        if (!file_helper::create(__data_file)) {
            LOG(ERROR) << "create data_file: " << __data_file << " failed" << std::endl;
            return false;
        }
        return true;
    }
    void remove_msg_file() {
        file_helper::remove(__data_file);
        file_helper::remove(__tmp_file);
    }
    bool insert(message_ptr& msg) {
        return __insert(__data_file, msg);
    }
    bool remove(message_ptr& msg) {
        // 1. 将msg的有效位改为0
        msg->mutable_payload()->set_valid("0");
        // 2. 对msg进行序列化
        std::string body = msg->payload().SerializeAsString();
        if (body.size() != msg->length()) {
            LOG(WARNING) << "unable to modify the data, the new data length != origin data length" << std::endl;
            return false;
        }
        // 3. 将序列化后的消息，写入到数据在文件中的指定位置（覆盖原有的数据）
        file_helper helper(__data_file);
        bool ret = helper.write(body.c_str(), msg->offset(), body.size());
        if (ret == false) {
            LOG(ERROR) << "modify data to queue_file failed" << std::endl;
            return false;
        }
        return true;
    }
    std::list<message_ptr> gc() {
        std::list<message_ptr> result_lst;
        // 1. 加载文件中所有的有效数据
        if (!__load(result_lst)) {
            LOG(WARNING) << "load valid origin data failed" << std::endl;
            return result_lst;
        }
        // 2. 将有效数据进行序列化，然后存储到临时文件中
        for (auto& msg : result_lst) {
            if (!__insert(__tmp_file, msg))
                return result_lst;
        }
        // 3. 删除原文件
        if (!file_helper::remove(__data_file)) {
            LOG(WARNING) << "remove origin datafile: " << __data_file << " failed" << std::endl;
            return result_lst;
        }
        // 4. 修改临时文件名
        if (!file_helper(__tmp_file).rename(__data_file)) {
            LOG(WARNING) << "rename tmpfile: " << __tmp_file << " failed" << std::endl;
            return result_lst;
        }
        // 5. 返回新的有效数据
        return result_lst;
    } // 垃圾回收
private:
    bool __load(std::list<message_ptr>& lst) {
        // 1. 加载文件中所有的有效数据
        file_helper data_file_helper(__data_file);
        size_t msg_size;
        size_t offset = 0;
        size_t fsize = data_file_helper.size();
        while (offset < fsize) {
            // 先读取4字节的长度
            if (!data_file_helper.read((char*)&msg_size, offset, 4)) {
                // 读取长度存到一个size_t里面去
                LOG(WARNING) << "read msg length fail!" << std::endl;
                return false;
            }
            offset += 4;
            std::string msg_body(msg_size, '\0');
            if (!data_file_helper.read(&msg_body[0], offset, msg_size)) {
                LOG(WARNING) << "read msg fail!" << std::endl;
                return false;
            }
            offset += msg_size;
            message_ptr msgp = std::make_shared<Message>();
            msgp->ParseFromString(msg_body);
            if (msgp->payload().valid() == "0")
                continue;
            lst.push_back(msgp); // 如果是无效消息就处理下一个，如果是有效的，就保存起来
        }
    }
    bool __insert(const std::string file_name, message_ptr& msg) {
        // 新增数据都是添加到文件末尾的
        // 1. 进行消息的序列化，获取到格式化后的消息
        std::string body = msg->payload().SerializeAsString();
        // 2. 获取文件的长度
        file_helper helper(file_name);
        size_t fsize = helper.size();
        // 3. 将数据写入文件的指定位置
        bool ret = helper.write(body.c_str(), fsize, body.size());
        if (ret == false) {
            LOG(ERROR) << "write data to " << file_name << " failed" << std::endl;
            return false;
        }
        // 4. 更新msg中的实际存储信息
        msg->set_offset(fsize);
        msg->set_length(body.size());
        return true;
    }
};
} // namespace hare_mq

#endif