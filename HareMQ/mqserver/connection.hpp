/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_CONNECTION__
#define __YUFC_CONNECTION__

#include "channel.hpp"

namespace hare_mq {
class connection {
private:
    muduo::net::TcpConnectionPtr __conn;
    ProtobufCodecPtr __codec;
    consumer_manager::ptr __cmp;
    virtual_host::ptr __host;
    thread_pool::ptr __pool;
    channel_manager::ptr __channels; //
public:
    using ptr = std::shared_ptr<connection>;
    connection(const virtual_host::ptr& host,
        const consumer_manager::ptr& cmp,
        const ProtobufCodecPtr& codec,
        const muduo::net::TcpConnectionPtr& conn,
        const thread_pool::ptr& pool)
        : __conn(conn)
        , __codec(codec)
        , __cmp(cmp)
        , __host(host)
        , __pool(pool)
        , __channels(std::make_shared<channel_manager>()) { }
    ~connection() = default;
    void open_channel(const openChannelRequestPtr& req) {
        // 1. 判断信道ID是否重复 2. 创建信道
        bool ret = __channels->open_channel(req->cid(), __host, __cmp, __codec, __conn, __pool); // bug found!
        if (ret == false)
            return basic_response(false, req->rid(), req->cid());
        // 3. 给客户端回复
        return basic_response(true, req->rid(), req->cid());
    }
    void close_channel(const closeChannelRequestPtr& req) {
        __channels->close_channel(req->cid());
        return basic_response(true, req->rid(), req->cid());
    } //
    channel::ptr select_channel(const std::string& cid) {
        return __channels->select_channel(cid);
    } //
private:
    void basic_response(bool ok, const std::string& rid, const std::string& cid) {
        basicCommonResponse resp;
        resp.set_rid(rid);
        resp.set_cid(cid);
        resp.set_ok(ok);
        __codec->send(__conn, resp); // 发送响应给客户端
    } //
};

class connection_manager {
private:
    std::mutex __mtx;
    std::unordered_map<muduo::net::TcpConnectionPtr, connection::ptr> __conns; //
public:
    using ptr = std::shared_ptr<connection_manager>;
    connection_manager() = default;
    ~connection_manager() = default;
    void new_connection(const virtual_host::ptr& host,
        const consumer_manager::ptr& cmp,
        const ProtobufCodecPtr& codec,
        const muduo::net::TcpConnectionPtr& conn,
        const thread_pool::ptr& pool) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __conns.find(conn);
        if (it != __conns.end()) // 已经有了
            return;
        auto self_conn = std::make_shared<connection>(host, cmp, codec, conn, pool);
        __conns.insert({ conn, self_conn });
    }
    void delete_connection(const muduo::net::TcpConnectionPtr& conn) {
        std::unique_lock<std::mutex> lock(__mtx);
        __conns.erase(conn);
    }
    connection::ptr select_connection(const muduo::net::TcpConnectionPtr& conn) {
        std::unique_lock<std::mutex> lock(__mtx);
        auto it = __conns.find(conn);
        if (it == __conns.end()) // 没找到
            return connection::ptr();
        return it->second;
    }
};

} // namespace hare_mq

#endif