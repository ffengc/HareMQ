

#ifndef __YUFC_DEMO_PROTOC_SERVER_USE_MUDUO__
#define __YUFC_DEMO_PROTOC_SERVER_USE_MUDUO__

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/protoc/codec.h"
#include "muduo/protoc/dispatcher.h"

#include "../log.hpp"
#include "request.pb.h"
#include <iostream>

class server {
public:
    typedef std::shared_ptr<yufc::translateRequest> translate_request_ptr;
    typedef std::shared_ptr<yufc::translateResponse> translate_response_ptr;
    typedef std::shared_ptr<yufc::addRequest> add_request_ptr;
    typedef std::shared_ptr<yufc::addResponse> add_response_ptr;

private:
    muduo::net::EventLoop __base_loop; // 回调
    muduo::net::TcpServer __server; // 服务器对象
    ProtobufDispatcher __dispatcher; // 请求分发器对象 -- 要向其中注册请求处理函数
    ProtobufCodec __codec; // protobuf协议处理器 -- 针对收到的请求数据进行protobuf协议处理
public:
    server(int port)
        : __server(&__base_loop, muduo::net::InetAddress("0.0.0.0", port), "server", muduo::net::TcpServer::kReusePort)
        , __dispatcher(std::bind(&server::onUnknownMessage,
              this, std::placeholders::_1,
              std::placeholders::_2,
              std::placeholders::_3))
        , __codec(std::bind(&ProtobufDispatcher::onProtobufMessage,
              &__dispatcher, std::placeholders::_1,
              std::placeholders::_2,
              std::placeholders::_3)) {
        // 注册业务请求处理函数
        __dispatcher.registerMessageCallback<yufc::translateRequest>(std::bind(&server::onTranslate,
            this, std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
        __dispatcher.registerMessageCallback<yufc::addRequest>(std::bind(&server::onAdd,
            this, std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
        // 设置消息回调
        __server.setMessageCallback(std::bind(&ProtobufCodec::onMessage, &__codec,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
        __server.setConnectionCallback(std::bind(&server::onConnection, this, std::placeholders::_1));
    }
    void start() {
        __server.start();
        __base_loop.loop();
    }

private:
    std::string translate(const std::string& str) {
        // 用个简单例子就行
        static std::unordered_map<std::string, std::string> __dict_map = {
            { "hello", "nihao" }, { "nihao", "hello" }
        };
        auto it = __dict_map.find(str); // 这里的str包含了\n，需要额外处理，不过这里只是为了学习使用服务器，不处理了
        if (it == __dict_map.end())
            return "unknown\n";
        return it->second;
    }
    void onTranslate(const muduo::net::TcpConnectionPtr& conn, const translate_request_ptr& message, muduo::Timestamp ts) {
        // 1. 提取message中的有效信息，也就是需要翻译的内容
        std::string req_msg = message->msg();
        // 2. 进行翻译，得到结果
        std::string rsp_msg = translate(req_msg);
        // 3. 组织protobuf的响应
        yufc::translateResponse resp;
        resp.set_msg(rsp_msg);
        // 4. 发送响应
        __codec.send(conn, resp); // 这里不是 conn->send(), 因为resp是需要序列化的，所以调用 __codec.send()
        // __codec.send() == 序列化 + conn->send()
    }
    void onAdd(const muduo::net::TcpConnectionPtr& conn, const add_request_ptr& message, muduo::Timestamp ts) {
        int num1 = message->num1();
        int num2 = message->num2();
        int result = num1 + num2;
        yufc::addResponse resp;
        resp.set_result(result);
        __codec.send(conn, resp);
    }
    void onUnknownMessage(const muduo::net::TcpConnectionPtr& conn, const MessagePtr& message, muduo::Timestamp ts) {
        LOG(INFO) << "onUnknownMessage: " << message->GetTypeName() << std::endl;
        conn->shutdown();
    }
    void onConnection(const muduo::net::TcpConnectionPtr& conn) {
        if (conn->connected())
            LOG(INFO) << "connected" << std::endl;
        else
            LOG(INFO) << "disconnected" << std::endl;
    }
};

#endif

int main() {
    server svr(8085);
    svr.start();
    return 0;
}