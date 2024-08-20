/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */
#include "connection.hpp"

void publish_client() {
    // 1. 实例化异步工作线程
    hare_mq::async_worker::ptr awp = std::make_shared<hare_mq::async_worker>();
    // 2. 实例化连接对象
    hare_mq::connection::ptr conn = std::make_shared<hare_mq::connection>("127.0.0.1", 8085, awp);
    // 3. 通过连接创建信道
    hare_mq::channel::ptr ch = conn->openChannel();
    // 4. 通过信道提供的服务完成所需
    //  4.1 声明一个交换机 exchange1, 类型为广播类型
    auto empty_map = std::unordered_map<std::string, std::string>();
    ch->declare_exchange("exchange1", hare_mq::ExchangeType::FANOUT, true, false, empty_map);
    //  4.2 声明一个队列 queue1
    ch->declare_queue("queue1", true, false, false, empty_map);
    //  4.3 声明一个队列 queue2
    ch->declare_queue("queue2", true, false, false, empty_map);
    //  4.4 绑定 queue1-exchange1, 且 binding_key 设置为 queue1
    ch->bind("exchange1", "queue1", "queue1");
    //  4.5 绑定 queue2-exchange1, 且 binding_key 设置为 news.music.#
    ch->bind("exchange1", "queue2", "news.music.#");
    // 5. 循环向交换机发布消息
    for (int i = 0; i < 10; ++i)
        ch->basic_publish("exchange1", nullptr, "Hello world-" + std::to_string(i));
    // 6. 关闭信道
    conn->closeChannel(ch);
}

int main() {
    publish_client();
    return 0;
}