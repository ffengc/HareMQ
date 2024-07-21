/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_DEMO_DICT_CLIENT_USE_MUDUO__
#define __YUFC_DEMO_DICT_CLIENT_USE_MUDUO__

#include "../log.hpp"
#include "muduo/base/CountDownLatch.h"
#include "muduo/net/EventLoopThread.h"
#include "muduo/net/TcpClient.h"
#include "muduo/net/TcpConnection.h"

/* 注意，客户端连接服务器是需要阻塞等待连接建立成功之后才返回的，所以才需要使用 CountDownLatch */

class translate_client {
private:
    muduo::CountDownLatch __latch;
    muduo::net::EventLoopThread __loop_thread;
    muduo::net::TcpClient __client;
    muduo::net::TcpConnectionPtr __conn;

private:
    // 连接成功的回调
    void onConnection(const muduo::net::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            // 如果连接建立成功了，就计数器--
            __latch.countDown();
            LOG(INFO) << "connection to server success" << std::endl;
            __conn = conn; // 保存这个连接
        } else {
            // 连接关闭
            LOG(INFO) << "connection to server end" << std::endl;
            __conn.reset(); // 清空
        }
    }
    // 收到服务器发来的消息的回调
    void onMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp) {
        std::cout << "server# " << buf->retrieveAllAsString() << std::endl;
    }

public:
    translate_client(const std::string& sip, int sport)
        : __latch(1)
        , __client(__loop_thread.startLoop(),
              muduo::net::InetAddress(sip, sport),
              "translate_client") {
        __client.setConnectionCallback(std::bind(&translate_client::onConnection,
            this, std::placeholders::_1));
        __client.setMessageCallback(std::bind(&translate_client::onMessage,
            this, std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
    }
    void connect() {
        __client.connect(); // 这里是立即返回的，但是我们需要控制阻塞等待！
        __latch.wait();
    }
    bool send(const std::string& mesg) {
        // 因为muduo里面的所有操作都是异步的，不知道什么时候可能连接都关闭了，所以是要判断的
        if (__conn->connected()) {
            __conn->send(mesg);
            return true;
        }
        return false;
    }
};

#endif

int main() {
    translate_client client("127.0.0.1", 8085);
    client.connect();
    while (1) {
        std::string buf;
        std::cin >> buf;
        client.send(buf);
    }
    return 0;
}