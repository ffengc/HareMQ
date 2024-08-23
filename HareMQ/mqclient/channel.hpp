/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_CLIENT_CHANNEL__
#define __YUFC_CLIENT_CHANNEL__

#include "../mqcommon/helper.hpp"
#include "../mqcommon/logger.hpp"
#include "../mqcommon/msg.pb.h"
#include "../mqcommon/protocol.pb.h"
#include "../mqcommon/thread_pool.hpp"
#include "consumer.hpp"
#include "muduo/net/TcpConnection.h"
#include "muduo/protoc/codec.h"
#include "muduo/protoc/dispatcher.h"
#include <condition_variable>
#include <mutex>

namespace hare_mq {
using ProtobufCodecPtr = std::shared_ptr<ProtobufCodec>;
using basicConsumeResponsePtr = std::shared_ptr<basicConsumeResponse>;
using basicCommonResponsePtr = std::shared_ptr<basicCommonResponse>;
using basicQueryResponsePtr = std::shared_ptr<basicQueryResponse>; //
class channel {
public:
    using ptr = std::shared_ptr<channel>; //
private:
    std::string __cid;
    muduo::net::TcpConnectionPtr __conn;
    ProtobufCodecPtr __codec;
    consumer::ptr __consumer;
    std::mutex __mtx;
    std::condition_variable __cv;
    std::unordered_map<std::string, basicCommonResponsePtr> __basic_resp; //
    std::unordered_map<std::string, basicQueryResponsePtr> __basic_query_resp; //
public:
    channel(const muduo::net::TcpConnectionPtr& conn, const ProtobufCodecPtr& codec)
        : __conn(conn)
        , __cid(uuid_helper::uuid())
        , __codec(codec) { }
    ~channel() {
        // 需要取消订阅
        /* note: 如果不取消订阅，也不会有问题，因为我们的服务端很完善，如果被释放，所有东西都会自动解除的 */
        basic_cancel();
    }
    bool open_server_channel() {
        openChannelRequest req;
        std::string rid = uuid_helper::uuid();
        req.set_rid(rid);
        req.set_cid(__cid);
        __codec->send(__conn, req);
        basicCommonResponsePtr resp = wait_response(rid);
        return resp->ok();
    }
    void close_server_channel() {
        closeChannelRequest req;
        std::string rid = uuid_helper::uuid();
        req.set_rid(rid);
        req.set_cid(__cid);
        __codec->send(__conn, req);
        basicCommonResponsePtr resp = wait_response(rid);
    }
    bool declare_exchange(const std::string& name,
        ExchangeType type,
        bool durable,
        bool auto_delete,
        const std::unordered_map<std::string, std::string>& args) {
        // 构造请求对象
        declareExchangeRequest req;
        std::string rid = uuid_helper::uuid();
        req.set_rid(rid);
        req.set_cid(__cid);
        req.set_exchange_name(name);
        req.set_exchange_type(type);
        req.set_durable(durable);
        req.set_auto_delete(auto_delete);
        auto m = map_helper::ConvertStdMapToProtoMap(args);
        req.mutable_args()->swap(m);
        // 向服务器发送请求
        __codec->send(__conn, req);
        // 等待响应
        basicCommonResponsePtr resp = wait_response(rid);
        return resp->ok();
    }
    void delete_exchange(const std::string& name) {
        std::string rid = uuid_helper::uuid();
        deleteExchangeRequest req;
        req.set_rid(rid);
        req.set_cid(__cid);
        req.set_exchange_name(name);
        __codec->send(__conn, req);
        basicCommonResponsePtr resp = wait_response(rid);
    }
    bool declare_queue(const std::string& qname,
        bool qdurable,
        bool qexclusive,
        bool qauto_delete,
        const std::unordered_map<std::string, std::string>& qargs) {
        std::string rid = uuid_helper::uuid();
        declareQueueRequest req;
        req.set_rid(rid);
        req.set_cid(__cid);
        req.set_queue_name(qname);
        req.set_durable(qdurable);
        req.set_exclusive(qexclusive);
        req.set_auto_delete(qauto_delete);
        auto m = map_helper::ConvertStdMapToProtoMap(qargs);
        req.mutable_args()->swap(m);
        __codec->send(__conn, req);
        basicCommonResponsePtr resp = wait_response(rid);
        return resp->ok();
    }
    void delete_queue(const std::string& name) {
        std::string rid = uuid_helper::uuid();
        deleteQueueRequest req;
        req.set_rid(rid);
        req.set_cid(__cid);
        req.set_queue_name(name);
        __codec->send(__conn, req);
        basicCommonResponsePtr resp = wait_response(rid);
    }
    bool bind(const std::string& ename, const std::string& qname, const std::string& key) {
        std::string rid = uuid_helper::uuid();
        bindRequest req;
        req.set_rid(rid);
        req.set_cid(__cid);
        req.set_exchange_name(ename);
        req.set_queue_name(qname);
        req.set_binding_key(key);
        __codec->send(__conn, req);
        basicCommonResponsePtr resp = wait_response(rid);
        return resp->ok();
    }
    void unbind(const std::string& ename, const std::string& qname) {
        std::string rid = uuid_helper::uuid();
        unbindRequest req;
        req.set_rid(rid);
        req.set_cid(__cid);
        req.set_exchange_name(ename);
        req.set_queue_name(qname);
        __codec->send(__conn, req);
        basicCommonResponsePtr resp = wait_response(rid);
    }
    void basic_publish(const std::string& ename, const BasicProperties* bp, const std::string& body) {
        basicPublishRequest req;
        std::string rid = uuid_helper::uuid();
        req.set_rid(rid);
        req.set_cid(__cid);
        req.set_body(body);
        req.set_exchange_name(ename);
        if (bp) {
            req.mutable_properties()->set_id(bp->id());
            req.mutable_properties()->set_delivery_mode(bp->delivery_mode());
            req.mutable_properties()->set_routing_key(bp->routing_key());
        }
        __codec->send(__conn, req);
        basicCommonResponsePtr resp = wait_response(rid);
    }
    void basic_ack(const std::string& msgid) {
        if (__consumer == nullptr) {
            LOG(ERROR) << "cannot find consumer info" << std::endl;
            return;
        }
        basicAckRequest req;
        std::string rid = uuid_helper::uuid();
        req.set_rid(rid);
        req.set_cid(__cid);
        req.set_queue_name(__consumer->qname); // fix bus
        req.set_message_id(msgid);
        __codec->send(__conn, req);
        basicCommonResponsePtr resp = wait_response(rid);
    }
    bool basic_consume(const std::string& consumer_tag, const std::string& queue_name, bool auto_ack, const consumer_callback& cb) {
        if (__consumer != nullptr) {
            // already subscribe another queue
            LOG(ERROR) << "the channel already subscribed another queue" << std::endl;
            return false;
        }
        basicConsumeRequest req;
        std::string rid = uuid_helper::uuid();
        req.set_rid(rid);
        req.set_cid(__cid);
        req.set_consumer_tag(consumer_tag);
        req.set_queue_name(queue_name);
        req.set_auto_ack(auto_ack);
        __codec->send(__conn, req);
        basicCommonResponsePtr resp = wait_response(rid);
        if (resp->ok() == false) {
            LOG(ERROR) << "subscribe queue failed!" << std::endl;
            return false;
        }
        __consumer = std::make_shared<consumer>(consumer_tag, queue_name, auto_ack, cb);
        return true;
    }
    void basic_cancel() {
        if (__consumer == nullptr)
            return;
        basicCancelRequest req;
        std::string rid = uuid_helper::uuid();
        req.set_rid(rid);
        req.set_cid(__cid);
        req.set_queue_name(__consumer->qname);
        req.set_consumer_tag(__consumer->tag);
        __codec->send(__conn, req);
        basicCommonResponsePtr resp = wait_response(rid);
        __consumer.reset();
    } //
    void basic_query() {
        basicQueryRequest req;
        std::string rid = uuid_helper::uuid();
        req.set_rid(rid);
        req.set_cid(__cid);
        __codec->send(__conn, req);
        basicQueryResponsePtr resp = wait_query_response(rid);
        return;
    } //
public:
    std::string cid() { return this->__cid; }
    void push_basic_response(const basicCommonResponsePtr& resp) {
        std::unique_lock<std::mutex> lock(__mtx);
        __basic_resp.insert({ resp->rid(), resp });
        __cv.notify_all(); // 唤醒 wait_response
    } // 连接收到响应向hashmap添加响应
    void push_basic_response(const basicQueryResponsePtr& resp) {
        std::unique_lock<std::mutex> lock(__mtx);
        __basic_query_resp.insert({ resp->rid(), resp });
        __cv.notify_all(); 
    }
    // 连接收到消息推送后，需要通过信道找到对应的消费者对象，通过回调函数进行消息处理
    void consume(const basicConsumeResponsePtr& resp) {
        // std::unique_lock<std::mutex> lock(__mtx); // 千千万万不能加锁！这个是线程调的！
        if (__consumer == nullptr) {
            LOG(ERROR) << "cannot find subscriber info" << std::endl;
            return;
        }
        if (__consumer->tag != resp->consumer_tag()) {
            LOG(ERROR) << "error response, '__consumer->tag != resp->consumer_tag()', consume mesg failed" << std::endl;
            return;
        }
        __consumer->callback(resp->consumer_tag(), resp->mutable_properties(), resp->body());
    } //
private:
    basicCommonResponsePtr wait_response(const std::string& rid) {
        std::unique_lock<std::mutex> lock(__mtx);
        __cv.wait(lock, [&rid, this]() {
            return __basic_resp.find(rid) != __basic_resp.end();
        });
        basicCommonResponsePtr bresp = __basic_resp[rid];
        __basic_resp.erase(rid);
        return bresp;
    } // 等待指定请求的响应
    basicQueryResponsePtr wait_query_response(const std::string& rid) {
        std::unique_lock<std::mutex> lock(__mtx);
        __cv.wait(lock, [&rid, this]() {
            return __basic_query_resp.find(rid) != __basic_query_resp.end();
        });
        basicQueryResponsePtr bresp = __basic_query_resp[rid];
        __basic_query_resp.erase(rid);
        std::cout << bresp->body() << std::endl;
        return bresp;
    }
};

class channel_manager {
private:
    std::mutex __mtx;
    std::unordered_map<std::string, channel::ptr> __channels; //
public:
    using ptr = std::shared_ptr<channel_manager>;
    channel_manager() = default;
    channel::ptr create_channel(muduo::net::TcpConnectionPtr& conn, ProtobufCodecPtr& codec) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto ch = std::make_shared<channel>(conn, codec);
        __channels.insert({ ch->cid(), ch });
        return ch;
    }
    void remove_channel(const std::string& cid) {
        std::unique_lock<std::mutex> lock(__mtx);
        __channels.erase(cid);
    }
    channel::ptr select_channel(const std::string& cid) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __channels.find(cid);
        if (it == __channels.end())
            return channel::ptr();
        return it->second;
    }
};

} // namespace hare_mq

#endif