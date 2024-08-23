/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_CLIENT_CONNECTION__
#define __YUFC_CLIENT_CONNECTION__

#include "../mqcommon/logger.hpp"
#include "../mqcommon/msg.pb.h"
#include "../mqcommon/protocol.pb.h"
#include "async_worker.hpp"
#include "channel.hpp"
#include "muduo/base/CountDownLatch.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"
#include "muduo/protoc/codec.h"
#include "muduo/protoc/dispatcher.h"

namespace hare_mq {
class connection {
public:
    using ptr = std::shared_ptr<connection>;
    using message_ptr = std::shared_ptr<google::protobuf::Message>;
    using basicCommonResponsePtr = std::shared_ptr<basicCommonResponse>;
    using basicConsumeResponsePtr = std::shared_ptr<basicConsumeResponse>;
    using basicQueryResponsePtr = std::shared_ptr<basicQueryResponse>; //
private:
    muduo::CountDownLatch __latch; // 实现同步的
    muduo::net::TcpConnectionPtr __conn; // 客户端对应的连接
    muduo::net::TcpClient __client; // 客户端
    ProtobufDispatcher __dispatcher; // 请求分发器
    ProtobufCodecPtr __codec; // 协议处理器
    async_worker::ptr __worker; // 异步工作控制
private:
    channel_manager::ptr __channel_manager; //
public:
    connection(const std::string& sip, int sport, const async_worker::ptr& worker)
        : __latch(1)
        , __worker(worker)
        , __channel_manager(std::make_shared<channel_manager>())
        , __client(worker->loop_thread.startLoop(), muduo::net::InetAddress(sip, sport), "Client")
        , __dispatcher(std::bind(&connection::onUnknownMessage, this,
              std::placeholders::_1,
              std::placeholders::_2,
              std::placeholders::_3))
        , __codec(std::make_shared<ProtobufCodec>(std::bind(&ProtobufDispatcher::onProtobufMessage, &__dispatcher,
              std::placeholders::_1,
              std::placeholders::_2,
              std::placeholders::_3))) {
        __dispatcher.registerMessageCallback<basicCommonResponse>(std::bind(&connection::commonResponse,
            this, std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
        __dispatcher.registerMessageCallback<basicConsumeResponse>(std::bind(&connection::consumeResponse,
            this, std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
        __dispatcher.registerMessageCallback<basicQueryResponse>(std::bind(&connection::queryRespone,
            this, std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
        __client.setMessageCallback(std::bind(&ProtobufCodec::onMessage, __codec,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
        __client.setConnectionCallback(std::bind(&connection::onConnection, this, std::placeholders::_1));
        // 连接服务器
        __client.connect();
        __latch.wait();
    }
    channel::ptr openChannel() {
        channel::ptr ch = __channel_manager->create_channel(__conn, __codec);
        // 给服务器发送信道创建请求
        if (!ch->open_server_channel()) {
            LOG(ERROR) << "open channel failed!" << std::endl;
            return channel::ptr();
        }
        return ch;
    }
    void closeChannel(const channel::ptr& ch) {
        ch->close_server_channel();
        __channel_manager->remove_channel(ch->cid());
    } //
private:
    void commonResponse(const muduo::net::TcpConnectionPtr& conn, const basicCommonResponsePtr& message, muduo::Timestamp ts) {
        // 1. 找到信道
        channel::ptr ch = __channel_manager->select_channel(message->cid());
        if (ch == nullptr) {
            LOG(ERROR) << "cannot find channel info" << std::endl;
            return;
        }
        // 2. 将得到的响应对象添加到信道的基础响应map中
        ch->push_basic_response(message);
    }
    void consumeResponse(const muduo::net::TcpConnectionPtr& conn, const basicConsumeResponsePtr& message, muduo::Timestamp ts) {
        // 1. 找到信道
        channel::ptr ch = __channel_manager->select_channel(message->cid());
        // 2. 封装异步任务，抛入到线程池
        __worker->pool.push([ch, message]() {
            ch->consume(message);
        });
    }
    void queryRespone(const muduo::net::TcpConnectionPtr& conn, const basicQueryResponsePtr& message, muduo::Timestamp ts) {
        channel::ptr ch = __channel_manager->select_channel(message->cid());
        if (ch == nullptr) {
            LOG(ERROR) << "cannot find channel info" << std::endl;
            return;
        }
        ch->push_basic_response(message);
    }
    void onUnknownMessage(const muduo::net::TcpConnectionPtr& conn, const MessagePtr& message, muduo::Timestamp ts) {
        LOG(INFO) << "unknown result: " << message->GetTypeName() << std::endl;
        conn->shutdown();
    }
    void onConnection(const muduo::net::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            __latch.countDown();
            __conn = conn;
            LOG(INFO) << "connected" << std::endl;
        } else
            LOG(INFO) << "disconnected" << std::endl;
    }
};

} // namespace hare_mq

#endif