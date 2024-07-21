/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_DEMO_DICT_SERVER_USE_MUDUO__
#define __YUFC_DEMO_DICT_SERVER_USE_MUDUO__

// #include "../../libs/muduo/include/muduo/net/EventLoop.h"
// #include "../../libs/muduo/include/muduo/net/TcpConnection.h"
// #include "../../libs/muduo/include/muduo/net/TcpServer.h"
#include "../log.hpp"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/net/TcpServer.h"
#include <unordered_map>

class translate_server {
private:
    muduo::net::EventLoop __base_loop; // 基本的事件循环(这个要传给server, 所以要放前面)
    muduo::net::TcpServer __server; // 服务器对象
private:
    // 新连接建立成功时的回调函数
    // 会在一个连接建立成功，以及关闭的时候被调用
    void onConnection(const muduo::net::TcpConnectionPtr& conn) {
        if (conn->connected() == true)
            LOG(INFO) << "new connection!" << std::endl;
        else
            LOG(INFO) << "connection close" << std::endl;
    }
    // 通信连接收到请求时的回调函数
    void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp) {
        std::string str = buf->retrieveAllAsString();
        LOG(INFO) << "recv a mesg: " << str << std::endl;
        std::string resp = translate(str);
        conn->send(resp); // 向客户端进行发送
    }
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

public:
    translate_server(int port)
        : __server(&__base_loop,
              muduo::net::InetAddress("0.0.0.0", port),
              "translate_server",
              muduo::net::TcpServer::kReusePort) {
        __server.setConnectionCallback(std::bind(&translate_server::onConnection,
            this, std::placeholders::_1)); // 设置回调
        __server.setMessageCallback(std::bind(&translate_server::onMessage,
            this, std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3)); // 设置回调
    }
    void start() {
        __server.start(); // 开始事件监听
        __base_loop.loop(); // 开始事件监控，这是一个死循环阻塞接口
    }
};

#endif

int main() {
    translate_server server(8085);
    server.start();
    return 0;
}