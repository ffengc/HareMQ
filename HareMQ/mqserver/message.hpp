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
/* 持久化管理 */
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
        if (file_helper(dir).exists() == false) { // 不存在才创建
            if (!file_helper::create_dir(dir)) {
                // 先把这个目录创建出来
                LOG(FATAL) << "message_mapper()->create_dir(): " << dir << " failed" << std::endl;
                abort();
            }
        }
        create_msg_file(); // 这个要放外面，如果放里面就不对了，有可能有目录但没文件的情况下，就没有被调用了
    }
    bool create_msg_file() {
        if (file_helper(__data_file).exists() == true)
            return true;
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
        // LOG(DEBUG) << "gc get mesg size: " << result_lst.size() << std::endl;
        // 2. 将有效数据进行序列化，然后存储到临时文件中
        file_helper::create(__tmp_file);
        for (auto& msg : result_lst) {
            if (!__insert(__tmp_file, msg))
                return result_lst;
        }
        // LOG(DEBUG) << "(for debug) file size: " << file_helper(__tmp_file).size() << std::endl;
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
            if (!data_file_helper.read((char*)&msg_size, offset, sizeof(size_t))) {
                // 读取长度存到一个size_t里面去
                LOG(WARNING) << "read msg length fail!" << std::endl;
                return false;
            }
            offset += sizeof(size_t);
            std::string msg_body(msg_size, '\0');
            if (!data_file_helper.read(&msg_body[0], offset, msg_size)) {
                LOG(WARNING) << "read msg fail!" << std::endl;
                return false;
            }
            offset += msg_size;
            message_ptr msgp = std::make_shared<Message>();
            msgp->mutable_payload()->ParseFromString(msg_body); // find a bug
            if (msgp->payload().valid() == "0")
                continue;
            lst.push_back(msgp); // 如果是无效消息就处理下一个，如果是有效的，就保存起来
        }
        return true;
    }
    bool __insert(const std::string file_name, message_ptr& msg) {
        // 新增数据都是添加到文件末尾的
        // 1. 进行消息的序列化，获取到格式化后的消息
        std::string body = msg->payload().SerializeAsString();
        // 2. 获取文件的长度
        file_helper helper(file_name);
        size_t fsize = helper.size();
        // 3. 将数据写入文件的指定位置, 先写内容长度(size_t), 再写入内容
        size_t msg_size = body.size();
        bool ret = helper.write((char*)&msg_size, fsize, sizeof(size_t));
        if (ret == false) {
            LOG(ERROR) << "write data length to " << file_name << " failed" << std::endl;
            return false;
        }
        ret = helper.write(body.c_str(), fsize + sizeof(size_t), body.size());
        if (ret == false) {
            LOG(ERROR) << "write data to " << file_name << " failed" << std::endl;
            return false;
        }
        // 4. 更新msg中的实际存储信息
        msg->set_offset(fsize + sizeof(size_t));
        msg->set_length(body.size());
        return true;
    }
};

/* 队列管理（上面是持久化，这里是内存的）*/
class queue_message {
private:
    std::mutex __mtx;
    std::string __queue_name; // 队列名称
    size_t __valid_count; // 有效消息数量
    size_t __total_count; // 总共消息数量
    message_mapper __mapper; // 持久化的句柄
    std::list<message_ptr> __msgs; // 待推送的消息
    std::unordered_map<std::string, message_ptr> __durable_msgs; // 待持久化的消息
    std::unordered_map<std::string, message_ptr> __wait_ack_msgs; // 待确认的消息
public:
    using ptr = std::shared_ptr<queue_message>;
    queue_message(const std::string& base_dir, const std::string& qname)
        : __mapper(base_dir, qname)
        , __queue_name(qname)
        , __valid_count(0)
        , __total_count(0) { }
    bool recovery() {
        // 恢复历史消息
        std::unique_lock<std::mutex> lock(__mtx);
        __msgs = __mapper.gc();
        for (auto& msg : __msgs) {
            __durable_msgs.insert({ msg->payload().properties().id(), msg });
        }
        __valid_count = __total_count = __msgs.size();
        return true;
    }
    bool insert(const BasicProperties* bp, const std::string& body, bool queue_durable) {
        /* DeliveryMode delivery_mode: 如果上层设置了bp, 则按照bp的去设置，否则按照delivery_mode的去设置*/
        // 1. 构造消息对象
        message_ptr msg = std::make_shared<Message>();
        msg->mutable_payload()->set_body(body);
        if (bp != nullptr) {
            DeliveryMode mode = queue_durable ? bp->delivery_mode() : DeliveryMode::UNDURABLE;
            msg->mutable_payload()->mutable_properties()->set_id(bp->id());
            msg->mutable_payload()->mutable_properties()->set_delivery_mode(mode);
            msg->mutable_payload()->mutable_properties()->set_routing_key(bp->routing_key());
        } else {
            DeliveryMode mode = queue_durable ? DeliveryMode::DURABLE : DeliveryMode::UNDURABLE;
            msg->mutable_payload()->mutable_properties()->set_id(uuid_helper::uuid());
            msg->mutable_payload()->mutable_properties()->set_delivery_mode(mode);
            msg->mutable_payload()->mutable_properties()->set_routing_key("");
        }
        std::unique_lock<std::mutex> lock(__mtx); // lock
        // 2. 判断是否需要持久化
        if (msg->payload().properties().delivery_mode() == DeliveryMode::DURABLE) {
            // 需要持久化
            msg->mutable_payload()->set_valid("1"); // 在持久化存储中表示数据有效
            // 这个valid字段也就是持久化才有用，如果不需要持久化，就没用了
            // 3. (持久化)
            bool ret = __mapper.insert(msg);
            if (ret == false) {
                LOG(ERROR) << "durable storage failed: " << body.c_str() << std::endl;
                return false;
            }
            __valid_count += 1; // 持久化信息的量+1
            __total_count += 1;
            __durable_msgs.insert({ msg->payload().properties().id(), msg });
        }
        // 4. 内存的管理
        __msgs.push_back(msg);
        return true;
    }
    bool remove(const std::string& msg_id) {
        std::unique_lock<std::mutex> lock(__mtx); // lock
        // 1. 从待确认队列中查找消息
        auto it = __wait_ack_msgs.find(msg_id);
        if (it == __wait_ack_msgs.end()) // 没找到这条消息
            return true;
        // 2. 根据消息的持久化模式，决定是否删除持久化消息
        if (it->second->payload().properties().delivery_mode() == DeliveryMode::DURABLE) {
            // 3. 删除持久化信息
            __mapper.remove(it->second);
            // 4. 删除内存中的信息
            __durable_msgs.erase(msg_id);
            __valid_count -= 1; // 持久化文件有效数量-=1
            // 判断是否需要垃圾回收
            this->gc(); // 内部会判断是否需要垃圾回收的
        }
        __wait_ack_msgs.erase(msg_id);
        return true;
    } // ack, 每次remove后要去检查是否需要gc
    message_ptr front() {
        std::unique_lock<std::mutex> lock(__mtx);
        if (__msgs.size() == 0)
            return message_ptr();
        // 从mesg中取出数据
        message_ptr msg = __msgs.front();
        __msgs.pop_front();
        // 将这个消息，向代确认的hashmap中放进去
        __wait_ack_msgs.insert({ msg->payload().properties().id(), msg });
        return msg;
    } // 获取队首消息
    size_t getable_count() {
        std::unique_lock<std::mutex> lock(__mtx);
        return __msgs.size();
    }
    size_t total_count() {
        std::unique_lock<std::mutex> lock(__mtx);
        return __total_count;
    }
    size_t durable_count() {
        return __durable_msgs.size();
    }
    size_t wait_ack_count() {
        std::unique_lock<std::mutex> lock(__mtx);
        return __wait_ack_msgs.size();
    }
    void clear() {
        std::unique_lock<std::mutex> lock(__mtx);
        __mapper.remove_msg_file();
        __msgs.clear();
        __durable_msgs.clear();
        __wait_ack_msgs.clear();
        __valid_count = __total_count = 0;
    }

private:
    bool gc_check() {
        // 判断当前状态是否需要进行垃圾回收
        // 持久化消息总量 > 2000 且其中有效比例 <50% 的时候进行垃圾回收
        if ((__total_count > 2000) && (__valid_count * 1.0 / __total_count < 0.5))
            return true;
        return false;
    }
    void gc() {
        // 进行垃圾回收，获取有效的消息链表
        if (gc_check() == false)
            return;
        auto new_valid_msgs = __mapper.gc();
        assert(new_valid_msgs.size() == __valid_count); // for debug
        // 更新每一条消息的实际存储位置
        for (auto& m : new_valid_msgs) {
            auto it = __durable_msgs.find(m->payload().properties().id());
            if (it == __durable_msgs.end()) {
                // 不应该出现这种情况，__durable_msgs里面的消息和new_valid_msgs的消息应该是相同的，都是有效消息
                LOG(ERROR) << "a msg after gc missed" << std::endl;
                __msgs.push_back(m); // 丢到消息的内存管理中去
                __durable_msgs.insert({ m->payload().properties().id(), m });
                continue;
            }
            // 更新每一条消息的实际存储位置
            it->second->set_offset(m->offset());
            it->second->set_length(m->length());
        }
        // 更新当前的有效消息数量 & 总的持久化消息数量
        __valid_count = __total_count = new_valid_msgs.size();
    } // 垃圾回收之后，更新map
};

class message_manager {
private:
    std::mutex __mtx;
    std::string __base_dir;
    std::unordered_map<std::string, queue_message::ptr> __queue_msgs; //  map
public:
    using ptr = std::shared_ptr<message_manager>;
    message_manager(const std::string& base_dir)
        : __base_dir(base_dir) { }
    void init_queue_msg(const std::string& qname) {
        queue_message::ptr qmp;
        { // lock
            std::unique_lock<std::mutex> lock(__mtx);
            auto it = __queue_msgs.find(qname);
            if (it != __queue_msgs.end())
                return;
            qmp = std::make_shared<queue_message>(__base_dir, qname);
            __queue_msgs.insert(std::make_pair(qname, qmp));
        }
        qmp->recovery(); // no lock
    } // 创建队列
    void destroy_queue_msg(const std::string& qname) {
        queue_message::ptr qmp;
        {
            std::unique_lock<std::mutex> lock(__mtx);
            auto it = __queue_msgs.find(qname);
            if (it == __queue_msgs.end()) // 没找到这个队列，直接返回
                return;
            qmp = it->second;
            __queue_msgs.erase(it);
        }
        qmp->clear();
    } // 销毁队列
    bool insert(const std::string& qname, BasicProperties* bp, const std::string& body, bool queue_durable) {
        queue_message::ptr qmp;
        {
            std::unique_lock<std::mutex> lock(__mtx);
            auto it = __queue_msgs.find(qname);
            if (it == __queue_msgs.end()) {
                LOG(ERROR) << "insert msg failed, no this queue: " << qname << std::endl;
                return false;
            }
            qmp = it->second;
        }
        return qmp->insert(bp, body, queue_durable);
    } // 向 qname 插入一个消息
    message_ptr front(const std::string& qname) {
        queue_message::ptr qmp;
        {
            std::unique_lock<std::mutex> lock(__mtx);
            auto it = __queue_msgs.find(qname);
            if (it == __queue_msgs.end()) {
                LOG(ERROR) << "get queue front failed, no this queue: " << qname << std::endl;
                return message_ptr();
            }
            qmp = it->second;
        }
        return qmp->front();
    } // 获取 qname 这个队列的队首消息
    void ack(const std::string& qname, const std::string msg_id) {
        queue_message::ptr qmp;
        {
            std::unique_lock<std::mutex> lock(__mtx);
            auto it = __queue_msgs.find(qname);
            if (it == __queue_msgs.end()) {
                LOG(ERROR) << "ack mesg failed, no this queue: " << qname << std::endl;
                return;
            }
            qmp = it->second;
        }
        qmp->remove(msg_id); // 确认就是删除
    } // 对 qname 中的 msg_id 进行确认
    size_t getable_count(const std::string& qname) {
        queue_message::ptr qmp;
        {
            std::unique_lock<std::mutex> lock(__mtx);
            auto it = __queue_msgs.find(qname);
            if (it == __queue_msgs.end()) {
                LOG(ERROR) << "error in getable_count(), no this queue: " << qname << std::endl;
                return 0;
            }
            qmp = it->second;
        }
        return qmp->getable_count();
    }
    size_t total_count(const std::string& qname) {
        queue_message::ptr qmp;
        {
            std::unique_lock<std::mutex> lock(__mtx);
            auto it = __queue_msgs.find(qname);
            if (it == __queue_msgs.end()) {
                LOG(ERROR) << "error in total_count(), no this queue: " << qname << std::endl;
                return 0;
            }
            qmp = it->second;
        }
        return qmp->total_count();
    }
    size_t durable_count(const std::string& qname) {
        queue_message::ptr qmp;
        {
            std::unique_lock<std::mutex> lock(__mtx);
            auto it = __queue_msgs.find(qname);
            if (it == __queue_msgs.end()) {
                LOG(ERROR) << "error in durable_count(), no this queue: " << qname << std::endl;
                return 0;
            }
            qmp = it->second;
        }
        return qmp->durable_count();
    }
    size_t wait_ack_count(const std::string& qname) {
        queue_message::ptr qmp;
        {
            std::unique_lock<std::mutex> lock(__mtx);
            auto it = __queue_msgs.find(qname);
            if (it == __queue_msgs.end()) {
                LOG(ERROR) << "error in wait_ack_count(), no this queue: " << qname << std::endl;
                return 0;
            }
            qmp = it->second;
        }
        return qmp->wait_ack_count();
    }
    void clear() {
        std::unique_lock<std::mutex> lock(__mtx);
        for (auto& q : __queue_msgs)
            q.second->clear();
    }
};

} // namespace hare_mq

#endif