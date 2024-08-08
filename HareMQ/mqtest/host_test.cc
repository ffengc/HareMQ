/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqserver/virtual_host.hpp"
#include <gtest/gtest.h>

using namespace hare_mq;

class host_test : public testing::Test {
public:
    void SetUp() override {
        std::unordered_map<std::string, std::string> empty_map = std::unordered_map<std::string, std::string>();
        __host = std::make_shared<virtual_host>("host1", "./host1/message/", "./host1/host1.db");
        __host->declare_exchange("exchange1", ExchangeType::DIRECT, true, false, empty_map);
        __host->declare_exchange("exchange2", ExchangeType::DIRECT, true, false, empty_map);
        __host->declare_exchange("exchange3", ExchangeType::DIRECT, true, false, empty_map);

        __host->declare_queue("queue1", true, false, false, empty_map);
        __host->declare_queue("queue2", true, false, false, empty_map);
        __host->declare_queue("queue3", true, false, false, empty_map);

        __host->bind("exchange1", "queue1", "news.music.#");
        __host->bind("exchange1", "queue2", "news.music.#");
        __host->bind("exchange1", "queue3", "news.music.#");

        __host->bind("exchange2", "queue1", "news.music.#");
        __host->bind("exchange2", "queue2", "news.music.#");
        __host->bind("exchange2", "queue3", "news.music.#");

        __host->bind("exchange3", "queue1", "news.music.#");
        __host->bind("exchange3", "queue2", "news.music.#");
        __host->bind("exchange3", "queue3", "news.music.#");

        __host->basic_publish("queue1", nullptr, "hello world-1");
        __host->basic_publish("queue1", nullptr, "hello world-2");
        __host->basic_publish("queue1", nullptr, "hello world-3");

        __host->basic_publish("queue2", nullptr, "hello world-1");
        __host->basic_publish("queue2", nullptr, "hello world-2");
        __host->basic_publish("queue2", nullptr, "hello world-3");

        __host->basic_publish("queue3", nullptr, "hello world-1");
        __host->basic_publish("queue3", nullptr, "hello world-2");
        __host->basic_publish("queue3", nullptr, "hello world-3");
    }
    void TearDown() override {
        __host->clear();
    }

public:
    virtual_host::ptr __host;
};

// 验证新增信息是否正常
TEST_F(host_test, init_test) {
    ASSERT_EQ(__host->exists_exchange("exchange1"), true);
    ASSERT_EQ(__host->exists_exchange("exchange2"), true);
    ASSERT_EQ(__host->exists_exchange("exchange3"), true);

    ASSERT_EQ(__host->exists_queue("queue1"), true);
    ASSERT_EQ(__host->exists_queue("queue2"), true);
    ASSERT_EQ(__host->exists_queue("queue3"), true);

    ASSERT_EQ(__host->exists_binding("exchange1", "queue1"), true);
    ASSERT_EQ(__host->exists_binding("exchange1", "queue2"), true);
    ASSERT_EQ(__host->exists_binding("exchange1", "queue3"), true);

    ASSERT_EQ(__host->exists_binding("exchange2", "queue1"), true);
    ASSERT_EQ(__host->exists_binding("exchange2", "queue2"), true);
    ASSERT_EQ(__host->exists_binding("exchange2", "queue3"), true);

    ASSERT_EQ(__host->exists_binding("exchange3", "queue1"), true);
    ASSERT_EQ(__host->exists_binding("exchange3", "queue2"), true);
    ASSERT_EQ(__host->exists_binding("exchange3", "queue3"), true);

    message_ptr msg1 = __host->basic_consume("queue1");
    ASSERT_EQ(msg1->payload().body(), "hello world-1");
    msg1 = __host->basic_consume("queue1");
    ASSERT_EQ(msg1->payload().body(), "hello world-2");
    msg1 = __host->basic_consume("queue1");
    ASSERT_EQ(msg1->payload().body(), "hello world-3");
    msg1 = __host->basic_consume("queue1");
    ASSERT_EQ(msg1, nullptr);
}

// 验证删除是否正常
TEST_F(host_test, remove_exchange_test) {
    ASSERT_EQ(__host->exists_exchange("exchange1"), true);
    __host->delete_exchange("exchange1");
    ASSERT_EQ(__host->exists_binding("exchange1", "queue1"), false);
    ASSERT_EQ(__host->exists_binding("exchange1", "queue2"), false);
    ASSERT_EQ(__host->exists_binding("exchange1", "queue3"), false);
}
TEST_F(host_test, remove_queue_test) {
    ASSERT_EQ(__host->exists_queue("queue1"), true);
    __host->delete_queue("queue1");
    ASSERT_EQ(__host->exists_binding("exchange1", "queue1"), false);
    ASSERT_EQ(__host->exists_binding("exchange2", "queue1"), false);
    ASSERT_EQ(__host->exists_binding("exchange3", "queue1"), false);
    message_ptr msg1 = __host->basic_consume("queue1");
    ASSERT_EQ(msg1, nullptr);
}

// 验证ack
TEST_F(host_test, ack_test) {
    message_ptr msg1 = __host->basic_consume("queue1");
    ASSERT_EQ(msg1->payload().body(), "hello world-1");
    __host->basic_ack("queue1", msg1->payload().properties().id());
    msg1 = __host->basic_consume("queue1");
    ASSERT_EQ(msg1->payload().body(), "hello world-2");
    __host->basic_ack("queue1", msg1->payload().properties().id());
    msg1 = __host->basic_consume("queue1");
    ASSERT_EQ(msg1->payload().body(), "hello world-3");
    __host->basic_ack("queue1", msg1->payload().properties().id());
    msg1 = __host->basic_consume("queue1");
    ASSERT_EQ(msg1, nullptr);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    auto res = RUN_ALL_TESTS();
    LOG(INFO) << "res: " << res << std::endl;
    return 0;
}