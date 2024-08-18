/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqserver/channel.hpp"

int main() {
    hare_mq::channel_manager::ptr cmp = std::make_shared<hare_mq::channel_manager>();
    cmp->open_channel("c1",
        std::make_shared<hare_mq::virtual_host>("host1", "./host1/message/", "./host1/host1.db"),
        std::make_shared<hare_mq::consumer_manager>(),
        hare_mq::ProtobufCodecPtr(),
        muduo::net::TcpConnectionPtr(),
        thread_pool::ptr());
    return 0;
}