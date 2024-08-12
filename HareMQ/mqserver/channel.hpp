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
#include "consumer.hpp"
#include "muduo/net/TcpConnection.h"
#include "muduo/protoc/codec.h"
#include "muduo/protoc/dispatcher.h"
#include "virtual_host.hpp"

namespace hare_mq {
class channel {
public:
    using ProtobufCodecPtr = std::shared_ptr<ProtobufCodec>; //
private:
    std::string __cid; // 信道标识
    consumer::ptr __consumer; // 在haremq中一个信道对应一个消费者，不一定有效，因为信道不一定是消费者关联的
    muduo::net::TcpConnectionPtr __conn; // 连接句柄
    ProtobufCodecPtr __codec; // 协议处理
    consumer_manager::ptr __cmp; // 消费者管理句柄
    virtual_host::ptr __host; // 虚拟机对象管理句柄
public:
    channel(const std::string& cid,
        const virtual_host::ptr& host,
        const consumer_manager::ptr& cmp,
        const ProtobufCodecPtr& codec,
        const muduo::net::TcpConnectionPtr conn) { }
    ~channel();
    // 交换机的声明和删除
    void declare_exchange();
    void delete_exchange(); 
    // 队列的声明和删除
    void declare_queue();
    void delete_queue();
    // 队列的绑定与解除绑定
    void bind();
    void unbind();
    // 消息的发布和确认
    void basic_publish();
    void basic_ack();
    // 订阅/取消订阅队列消息
    void basic_consume();
    void basic_cancel();
};
} // namespace hare_mq

#endif