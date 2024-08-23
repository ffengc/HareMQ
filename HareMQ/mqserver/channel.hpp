/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_CHANNEL__
#define __YUFC_CHANNEL__

#include "../mqcommon/helper.hpp"
#include "../mqcommon/logger.hpp"
#include "../mqcommon/msg.pb.h"
#include "../mqcommon/protocol.pb.h"
#include "../mqcommon/thread_pool.hpp"
#include "consumer.hpp"
#include "muduo/net/TcpConnection.h"
#include "muduo/protoc/codec.h"
#include "muduo/protoc/dispatcher.h"
#include "route.hpp"
#include "virtual_host.hpp"

namespace hare_mq {
using ProtobufCodecPtr = std::shared_ptr<ProtobufCodec>;
using openChannelRequestPtr = std::shared_ptr<openChannelRequest>;
using closeChannelRequestPtr = std::shared_ptr<closeChannelRequest>;
using declareExchangeRequestPtr = std::shared_ptr<declareExchangeRequest>;
using deleteExchangeRequestPtr = std::shared_ptr<deleteExchangeRequest>;
using declareQueueRequestPtr = std::shared_ptr<declareQueueRequest>;
using deleteQueueRequestPtr = std::shared_ptr<deleteQueueRequest>;
using bindRequestPtr = std::shared_ptr<bindRequest>;
using unbindRequestPtr = std::shared_ptr<unbindRequest>;
using basicPublishRequestPtr = std::shared_ptr<basicPublishRequest>;
using basicAckRequestPtr = std::shared_ptr<basicAckRequest>;
using basicConsumeRequestPtr = std::shared_ptr<basicConsumeRequest>;
using basicCancelRequestPtr = std::shared_ptr<basicCancelRequest>;
using basicQueryRequestPtr = std::shared_ptr<basicQueryRequest>;
using basicCommonResponsePtr = std::shared_ptr<basicCommonResponse>; //
class channel {
public:
    using ptr = std::shared_ptr<channel>; //
private:
    std::string __cid; // 信道标识
    consumer::ptr __consumer; // 在haremq中一个信道对应一个消费者，不一定有效，因为信道不一定是消费者关联的
    muduo::net::TcpConnectionPtr __conn; // 连接句柄
    ProtobufCodecPtr __codec; // 协议处理
    consumer_manager::ptr __cmp; // 消费者管理句柄
    virtual_host::ptr __host; // 虚拟机对象管理句柄
    thread_pool::ptr __pool; // 异步的线程池
private:
    void basic_response(bool ok, const std::string& rid, const std::string& cid) {
        basicCommonResponse resp;
        resp.set_rid(rid);
        resp.set_cid(cid);
        resp.set_ok(ok);
        __codec->send(__conn, resp); // 发送响应给客户端
    } //
    void consume(const std::string& qname) {
        // 线程池的回调：向指定队列的订阅者去推送消息
        // 1. 从队列中取出一条消息
        message_ptr mp = __host->basic_consume(qname);
        if (mp == nullptr) {
            LOG(ERROR) << "asyn run 'void consume(const std::string& qname)' failed, there's no mesg in the queue: " << qname << std::endl;
            return;
        }
        // 2. 从队列的订阅者中取出一个订阅者
        consumer::ptr cp = __cmp->choose(qname); // 这个是在发布消息，不能找 __consumer，__consumer是这个channel作为消费者的时候才有用，我现在是在发布消息
        // 3. 调用订阅者对应的消息处理函数，实现消息的推送
        if (cp == nullptr) {
            LOG(ERROR) << "asyn run 'void consume(const std::string& qname)' failed, the queue has no consumers (nobody subscribed this queue)"
                       << qname << std::endl;
            return;
        }
        cp->callback(cp->tag, mp->mutable_payload()->mutable_properties(), mp->payload().body());
        // 4. 判断如果订阅者如果自动ack，则不需要等待确认，直接删除消息，否则需要等待外部收到消息确认后再删除
        if (cp->auto_ack)
            __host->basic_ack(qname, mp->payload().properties().id());
    }
    void consume_cb(const std::string& tag, const BasicProperties* bp, const std::string& body) {
        // 这个是消费者的回调，也就是说，消费一条信息，具体是如何消费
        // __cmp->create(req->consumer_tag(), req->queue_name(), req->auto_ack(), /*?*/);
        // 需要和这个保持一致: consumer.hpp
        //      using consumer_callback = std::function<void(const std::string&, const BasicProperties*, const std::string&)>;
        // 那推送一条消息给客户端，具体是做什么？就是组织一个响应的格式: basicConsumeResponse
        basicConsumeResponse resp;
        resp.set_cid(__cid);
        resp.set_body(body);
        resp.set_consumer_tag(tag);
        if (bp) {
            resp.mutable_properties()->set_id(bp->id());
            resp.mutable_properties()->set_delivery_mode(bp->delivery_mode());
            resp.mutable_properties()->set_routing_key(bp->routing_key());
        }
        __codec->send(__conn, resp);
    }

public:
    channel(const std::string& cid,
        const virtual_host::ptr& host,
        const consumer_manager::ptr& cmp,
        const ProtobufCodecPtr& codec,
        const muduo::net::TcpConnectionPtr conn,
        const thread_pool::ptr& pool)
        : __cid(cid)
        , __conn(conn)
        , __codec(codec)
        , __cmp(cmp)
        , __host(host)
        , __pool(pool) { }
    ~channel() {
        if (__consumer != nullptr)
            __cmp->remove(__consumer->tag, __consumer->qname); // 删除这个队列相关连的消费者
    }
    // 交换机的声明和删除
    void declare_exchange(const declareExchangeRequestPtr& req) {
        // 处理请求
        auto req_map = map_helper::ConvertProtoMapToStdMap(req->args());
        auto res = __host->declare_exchange(req->exchange_name(),
            req->exchange_type(),
            req->durable(),
            req->auto_delete(),
            req_map);
        // 构建响应
        basic_response(res, req->rid(), req->cid());
    }
    void delete_exchange(const deleteExchangeRequestPtr& req) {
        __host->delete_exchange(req->exchange_name());
        basic_response(true, req->rid(), req->cid());
    }
    // 队列的声明和删除
    void declare_queue(const declareQueueRequestPtr& req) {
        auto req_map = map_helper::ConvertProtoMapToStdMap(req->args());
        bool ret = __host->declare_queue(req->queue_name(),
            req->durable(),
            req->exclusive(), req->auto_delete(), req_map);
        if (ret == false)
            return basic_response(false, req->rid(), req->cid());
        __cmp->init_queue_consumer(req->queue_name()); // 初始化队列消费者管理句柄
        return basic_response(true, req->rid(), req->cid());
    }
    void delete_queue(const deleteQueueRequestPtr& req) {
        __cmp->destroy_queue_consumer(req->queue_name());
        __host->delete_queue(req->queue_name());
        return basic_response(true, req->rid(), req->cid());
    }
    // 队列的绑定与解除绑定
    void bind(const bindRequestPtr& req) {
        bool ret = __host->bind(req->exchange_name(), req->queue_name(), req->binding_key());
        return basic_response(ret, req->rid(), req->cid());
    }
    void unbind(const unbindRequestPtr& req) {
        __host->unbind(req->exchange_name(), req->queue_name());
        return basic_response(true, req->rid(), req->cid());
    }
    // 消息的发布和确认
    void basic_publish(const basicPublishRequestPtr& req) {
        // 1. 判断交换机是否存在
        auto ep = __host->select_exchange(req->exchange_name());
        if (ep == nullptr) // 没找到这台交换机
            return basic_response(false, req->rid(), req->cid());
        // 2. 进行路由（判断消息可以发布到交换机绑定的哪一个队列中去）
        msg_queue_binding_map mqbm = __host->exchange_bindings(req->exchange_name());
        BasicProperties* properties = nullptr;
        std::string routing_key;
        if (req->has_properties()) {
            properties = req->mutable_properties();
            routing_key = properties->routing_key();
        } // 因为不一定有 properties() 这个字段，所以要在外面先判断一下
        for (const auto& e : mqbm) {
            // 2.1 判断mqbm里面哪些队列是可以发的
            if (router::route(ep->type, routing_key, e.second->binding_key)) {
                // 3. 将消息添加到队列中（添加消息的管理）
                __host->basic_publish(e.first, properties, req->body());
                // 4. 向线程池中添加一个消息消费任务（向指定队列的订阅者去推送消息）
                auto task = std::bind(&channel::consume, this, e.first);
                __pool->push(task);
            }
        }
        return basic_response(true, req->rid(), req->cid());
    }
    void basic_ack(const basicAckRequestPtr& req) {
        __host->basic_ack(req->queue_name(), req->message_id()); // ack this mesg
        return basic_response(true, req->rid(), req->cid());
    }
    // 订阅/取消订阅队列消息
    void basic_consume(const basicConsumeRequestPtr& req) {
        // 1. 判断队列是否存在
        bool ret = __host->exists_queue(req->queue_name());
        if (ret == false)
            return basic_response(false, req->rid(), req->cid());
        // 2. 创建队列的消费者
        auto cb = std::bind(&channel::consume_cb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
        // 创建了消费者之后，当前的 channel 就是一个消费者
        __consumer = __cmp->create(req->consumer_tag(), req->queue_name(), req->auto_ack(), /*important*/ cb);
        return basic_response(true, req->rid(), req->cid());
    }
    void basic_cancel(const basicCancelRequestPtr& req) {
        __cmp->remove(req->consumer_tag(), req->queue_name());
        return basic_response(false, req->rid(), req->cid());
    }
    void basic_query(const basicQueryRequestPtr& req) {
        std::string ret = __host->basic_query();
        basicQueryResponse resp;
        resp.set_rid(req->rid());
        resp.set_cid(__cid);
        resp.set_body(ret);
        __codec->send(__conn, resp);
    }
};

class channel_manager {
private:
    std::unordered_map<std::string, channel::ptr> __channels;
    std::mutex __mtx; //
public:
    using ptr = std::shared_ptr<channel_manager>;
    channel_manager() = default;
    ~channel_manager() = default;
    bool open_channel(const std::string& cid,
        const virtual_host::ptr& host,
        const consumer_manager::ptr& cmp,
        const ProtobufCodecPtr& codec,
        const muduo::net::TcpConnectionPtr conn,
        const thread_pool::ptr& pool) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __channels.find(cid);
        if (it != __channels.end())
            return false;
        auto ch = std::make_shared<channel>(cid, host, cmp, codec, conn, pool);
        __channels.insert({ cid, ch });
        return true;
    }
    void close_channel(const std::string& cid) {
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