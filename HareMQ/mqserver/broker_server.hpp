/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_BROKER_SERVER__
#define __YUFC_BROKER_SERVER__

#include "muduo/base/Logging.h"
#include "muduo/base/Mutex.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/TcpServer.h"
#include "muduo/protoc/codec.h"
#include "muduo/protoc/dispatcher.h"

#include "../mqcommon/logger.hpp"
#include "../mqcommon/msg.pb.h"
#include "../mqcommon/protocol.pb.h"
#include "../mqcommon/thread_pool.hpp"
#include "connection.hpp"
#include "consumer.hpp"
#include "virtual_host.hpp"
#include <pwd.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

namespace hare_mq {
#define DBFILE_PATH "/meta.db"
#define HOST_NAME "MyVirtualHost"
class BrokerServer {
private:
    // server
    muduo::net::EventLoop __base_loop; // 回调
    muduo::net::TcpServer __server; // 服务器对象
    ProtobufDispatcher __dispatcher; // 请求分发器对象 -- 要向其中注册请求处理函数
    ProtobufCodecPtr __codec; // protobuf协议处理器 -- 针对收到的请求数据进行protobuf协议处理
    // broker data
    virtual_host::ptr __virtual_host;
    consumer_manager::ptr __consumer_manager;
    connection_manager::ptr __connection_manager;
    thread_pool::ptr __thread_pool; //
public:
    BrokerServer(int port, const std::string& basedir)
        : __server(&__base_loop, muduo::net::InetAddress("0.0.0.0", port), "server", muduo::net::TcpServer::kReusePort)
        , __dispatcher(std::bind(&BrokerServer::onUnknownMessage,
              this, std::placeholders::_1,
              std::placeholders::_2,
              std::placeholders::_3))
        , __codec(std::make_shared<ProtobufCodec>(std::bind(&ProtobufDispatcher::onProtobufMessage,
              &__dispatcher, std::placeholders::_1,
              std::placeholders::_2,
              std::placeholders::_3)))
        , __virtual_host(std::make_shared<virtual_host>(HOST_NAME, basedir, basedir + DBFILE_PATH))
        , __consumer_manager(std::make_shared<consumer_manager>())
        , __connection_manager(std::make_shared<connection_manager>())
        , __thread_pool(std::make_shared<thread_pool>()) {
        // 针对历史消息中的所有队列，别忘了去初始化队列的消费者管理句柄
        queue_map qm = __virtual_host->all_queues();
        for (const auto& e : qm)
            __consumer_manager->init_queue_consumer(e.first);
        // 注册业务请求处理函数
        __dispatcher.registerMessageCallback<openChannelRequest>(std::bind(&BrokerServer::on_openChannel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<closeChannelRequest>(std::bind(&BrokerServer::on_closeChannel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<declareExchangeRequest>(std::bind(&BrokerServer::on_declareExchange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<deleteExchangeRequest>(std::bind(&BrokerServer::on_deleteExchange, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<declareQueueRequest>(std::bind(&BrokerServer::on_declareQueue, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<deleteQueueRequest>(std::bind(&BrokerServer::on_deleteQueue, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<bindRequest>(std::bind(&BrokerServer::on_bind, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<unbindRequest>(std::bind(&BrokerServer::on_unbind, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<basicPublishRequest>(std::bind(&BrokerServer::on_basicPublish, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<basicAckRequest>(std::bind(&BrokerServer::on_basicAck, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<basicConsumeRequest>(std::bind(&BrokerServer::on_basicConsume, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<basicCancelRequest>(std::bind(&BrokerServer::on_basicCancel, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        __dispatcher.registerMessageCallback<basicQueryRequest>(std::bind(&BrokerServer::on_basicQuery, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
        // 设置消息回调
        __server.setMessageCallback(std::bind(&ProtobufCodec::onMessage, __codec,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
        __server.setConnectionCallback(std::bind(&BrokerServer::onConnection, this, std::placeholders::_1));
    }
    void start() {
        printServerInfo();
        __server.start();
        __base_loop.loop();
    } //
private:
    void printServerInfo() {
        auto listenAddr = __server.ipPort(); // Ensure you have a method or accessor to get this info from TcpServer
        auto startTime = std::time(nullptr);
        std::string startTimeStr = std::ctime(&startTime);
        auto uid = getuid();
        struct passwd* pw = getpwuid(uid);
        std::string userName = (pw ? pw->pw_name : "Unknown User");
        pid_t pid = getpid();
        LOG(INFO) << std::endl
                  << "------------------- Server Start --------------------" << std::endl
                  << "IP Address and Port: " << listenAddr << std::endl // make sure listenAddr provides correct format
                  << "Start Time: " << startTimeStr
                  << "User: " << userName << std::endl
                  << "Process ID: " << std::to_string(pid) << std::endl
                  << "-------------------------------------------------------" << std::endl;
    } //
private:
    // list all the request
    // 打开信道请求
    void on_openChannel(const muduo::net::TcpConnectionPtr& conn, const openChannelRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: openChannelRequest" << std::endl;
        return new_conn->open_channel(message);
    }
    // 关闭信道请求
    void on_closeChannel(const muduo::net::TcpConnectionPtr& conn, const closeChannelRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: closeChannelRequest" << std::endl;
        return new_conn->close_channel(message);
    }
    // 声明交换机请求
    void on_declareExchange(const muduo::net::TcpConnectionPtr& conn, const declareExchangeRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        channel::ptr cp = new_conn->select_channel(message->cid());
        if (cp == nullptr) {
            LOG(WARNING) << "unknown channel in this connection" << std::endl;
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: declareExchangeRequest" << std::endl;
        return cp->declare_exchange(message);
    }
    // 删除交换机请求
    void on_deleteExchange(const muduo::net::TcpConnectionPtr& conn, const deleteExchangeRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        channel::ptr cp = new_conn->select_channel(message->cid());
        if (cp == nullptr) {
            LOG(WARNING) << "unknown channel in this connection" << std::endl;
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: deleteExchangeRequest" << std::endl;
        return cp->delete_exchange(message);
    }
    // 声明队列请求
    void on_declareQueue(const muduo::net::TcpConnectionPtr& conn, const declareQueueRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        channel::ptr cp = new_conn->select_channel(message->cid());
        if (cp == nullptr) {
            LOG(WARNING) << "unknown channel in this connection" << std::endl;
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: declareQueueRequest" << std::endl;
        return cp->declare_queue(message);
    }
    // 删除队列请求
    void on_deleteQueue(const muduo::net::TcpConnectionPtr& conn, const deleteQueueRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        channel::ptr cp = new_conn->select_channel(message->cid());
        if (cp == nullptr) {
            LOG(WARNING) << "unknown channel in this connection" << std::endl;
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: deleteQueueRequest" << std::endl;
        return cp->delete_queue(message);
    }
    // 绑定请求
    void on_bind(const muduo::net::TcpConnectionPtr& conn, const bindRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        channel::ptr cp = new_conn->select_channel(message->cid());
        if (cp == nullptr) {
            LOG(WARNING) << "unknown channel in this connection" << std::endl;
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: bindRequest" << std::endl;
        return cp->bind(message);
    }
    // 解绑请求
    void on_unbind(const muduo::net::TcpConnectionPtr& conn, const unbindRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        channel::ptr cp = new_conn->select_channel(message->cid());
        if (cp == nullptr) {
            LOG(WARNING) << "unknown channel in this connection" << std::endl;
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: unbindRequest" << std::endl;
        return cp->unbind(message);
    }
    // 消息发布
    void on_basicPublish(const muduo::net::TcpConnectionPtr& conn, const basicPublishRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        channel::ptr cp = new_conn->select_channel(message->cid());
        if (cp == nullptr) {
            LOG(WARNING) << "unknown channel in this connection" << std::endl;
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: basicPublishRequest" << std::endl;
        return cp->basic_publish(message);
    }
    // 消息确认
    void on_basicAck(const muduo::net::TcpConnectionPtr& conn, const basicAckRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        channel::ptr cp = new_conn->select_channel(message->cid());
        if (cp == nullptr) {
            LOG(WARNING) << "unknown channel in this connection" << std::endl;
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: basicAckRequest" << std::endl;
        return cp->basic_ack(message);
    }
    // 消息订阅
    void on_basicConsume(const muduo::net::TcpConnectionPtr& conn, const basicConsumeRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        channel::ptr cp = new_conn->select_channel(message->cid());
        if (cp == nullptr) {
            LOG(WARNING) << "unknown channel in this connection" << std::endl;
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: basicConsumeRequest" << std::endl;
        return cp->basic_consume(message);
    }
    // 取消订阅
    void on_basicCancel(const muduo::net::TcpConnectionPtr& conn, const basicCancelRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        channel::ptr cp = new_conn->select_channel(message->cid());
        if (cp == nullptr) {
            LOG(WARNING) << "unknown channel in this connection" << std::endl;
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: basicCancelRequest" << std::endl;
        return cp->basic_cancel(message);
    }
    // 查询
    void on_basicQuery(const muduo::net::TcpConnectionPtr& conn, const basicQueryRequestPtr& message, muduo::Timestamp ts) {
        connection::ptr new_conn = __connection_manager->select_connection(conn);
        if (new_conn == nullptr) {
            LOG(WARNING) << "unknown connection" << std::endl;
            conn->shutdown();
            return;
        }
        channel::ptr cp = new_conn->select_channel(message->cid());
        if (cp == nullptr) {
            LOG(WARNING) << "unknown channel in this connection" << std::endl;
            return;
        }
        LOG(REQUEST) << "<from " << conn->peerAddress().toIpPort() << "> Request: basicQueryRequest" << std::endl;
        return cp->basic_query(message);
    }
    // 未知请求
    void onUnknownMessage(const muduo::net::TcpConnectionPtr& conn, const MessagePtr& message, muduo::Timestamp ts) {
        LOG(WARNING) << "onUnknownMessage: " << message->GetTypeName() << std::endl;
        conn->shutdown();
    }
    // 连接
    void printConnectionInfo(const muduo::net::TcpConnectionPtr& conn) {
        std::string connName = conn->name();
        std::string localIp = conn->localAddress().toIpPort();
        std::string peerIp = conn->peerAddress().toIpPort();
        LOG(INFO) << std::endl
                  << "New Connection: " << std::endl
                  << "Connection Name: " << connName << std::endl
                  << "Local IP: " << localIp << std::endl
                  << "Peer IP: " << peerIp << std::endl;
    }
    void onConnection(const muduo::net::TcpConnectionPtr& conn) {
        if (conn->connected()) {
            LOG(INFO) << "connected" << std::endl;
            printConnectionInfo(conn);
            __connection_manager->new_connection(__virtual_host, __consumer_manager, __codec, conn, __thread_pool);
        } else {
            LOG(INFO) << "disconnected" << std::endl;
            __connection_manager->delete_connection(conn);
        }
    }
};
} // namespace hare_mq

#endif