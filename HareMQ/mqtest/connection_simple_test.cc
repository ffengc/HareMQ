/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqserver/connection.hpp"
using namespace hare_mq;
int main() {
    auto cm = std::make_shared<connection_manager>();
    cm->new_connection(std::make_shared<hare_mq::virtual_host>("host1", "./host1/message/", "./host1/host1.db"),
        std::make_shared<hare_mq::consumer_manager>(),
        hare_mq::ProtobufCodecPtr(),
        muduo::net::TcpConnectionPtr(),
        thread_pool::ptr());
    return 0;
}