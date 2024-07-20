

#ifndef __YUFC_DEMO_PROTOC_CLIENT_USE_MUDUO__
#define __YUFC_DEMO_PROTOC_CLIENT_USE_MUDUO__

#include "muduo/base/CountDownLatch.h"
#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"
#include "muduo/protoc/codec.h"
#include "muduo/protoc/dispatcher.h"

#include "../log.hpp"
#include "request.pb.h"

class client {
public:
    typedef std::shared_ptr<google::protobuf::Message> message_ptr;
    typedef std::shared_ptr<yufc::addResponse> add_response_ptr;
    typedef std::shared_ptr<yufc::translateResponse> translate_response_ptr;

private:
    muduo::CountDownLatch __latch; // 实现同步的
    muduo::net::EventLoopThread __loop_thread; // 异步循环处理线程
    muduo::net::TcpConnectionPtr __conn; // 客户端对应的连接
    muduo::net::TcpClient __client; // 客户端
    ProtobufDispatcher __dispatcher; // 请求分发器
    ProtobufCodec __codec; // 协议处理器
public:
    client(const std::string& sip, int sport)
        : __latch(1)
        , __client(__loop_thread.startLoop(), muduo::net::InetAddress(sip, sport), "client")
        , __dispatcher(std::bind(&client::onUnknownMessage, this,
              std::placeholders::_1,
              std::placeholders::_2,
              std::placeholders::_3))
        , __codec(std::bind(&ProtobufDispatcher::onProtobufMessage, &__dispatcher,
              std::placeholders::_1,
              std::placeholders::_2,
              std::placeholders::_3)) {
        // 注册
        __dispatcher.registerMessageCallback<yufc::translateResponse>(std::bind(&client::onTranslate,
            this, std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
        __dispatcher.registerMessageCallback<yufc::addResponse>(std::bind(&client::onAdd,
            this, std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
        // 设置消息回调
        __client.setMessageCallback(std::bind(&ProtobufCodec::onMessage, &__codec,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
        __client.setConnectionCallback(std::bind(&client::onConnection, this, std::placeholders::_1));
    }
    void connect() {
        __client.connect();
        __latch.wait(); // 阻塞等待，直到建立成功
    }
    void translate(const std::string& mesg) {
        yufc::translateRequest req; // 请求对象
        req.set_msg(mesg);
        send(&req);
    }
    void add(int num1, int num2) {
        yufc::addRequest req; // 请求对象
        req.set_num1(num1);
        req.set_num2(num2);
        send(&req);
    }

private:
    bool send(const google::protobuf::Message* message) {
        /**
         * 这里需要好好理解: google::protobuf::Message* 是一个父类指针
         * 传递过来的是: yufc::translateRequest* 或者是 yufc::addRequest* 是子类的指针
         * 然后send()里面就会调用父类指针指向的子类对象的虚函数
         * 这样就能同时让 translate() 和 add() 都调用 send() 了，不用写两个send
         */
        if (__conn->connected()) {
            __codec.send(__conn, *message); // 改成利用__codec来发送
            return true;
        }
        return false;
    }
    void onUnknownMessage(const muduo::net::TcpConnectionPtr& conn, const MessagePtr& message, muduo::Timestamp ts) {
        LOG(INFO) << "unknown result: " << message->GetTypeName() << std::endl;
        conn->shutdown();
    }
    void onTranslate(const muduo::net::TcpConnectionPtr& conn, const translate_response_ptr& message, muduo::Timestamp ts) {
        LOG(INFO) << "translate result: " << message->msg() << std::endl;
    }
    void onAdd(const muduo::net::TcpConnectionPtr& conn, const add_response_ptr& message, muduo::Timestamp ts) {
        LOG(INFO) << "add result: " << message->result() << std::endl;
    }
    void onConnection(const muduo::net::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            __latch.countDown();
            __conn = conn;
            LOG(INFO) << "connected" << std::endl;
        }
        else
            LOG(INFO) << "disconnected" << std::endl;
    }
};

#endif

int main() {
    client clt("127.0.0.1", 8085);
    clt.connect();
    clt.translate("hello");
    clt.add(11, 22);
    sleep(1);
    return 0;
}