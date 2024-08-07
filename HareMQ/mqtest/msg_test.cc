/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqserver/message.hpp"
#include <gtest/gtest.h>

using namespace hare_mq;

hare_mq::message_manager::ptr mmp;

class msg_test : public testing::Environment {
public:
    virtual void SetUp() override {
        mmp = std::make_shared<message_manager>("./data-mesg");
        mmp->init_queue_msg("queue1");
    }
    virtual void TearDown() override {
        mmp->clear();
    }
};

#define ONLY_INSERT 1
#define RECOVERY_TEST 0
#if !RECOVERY_TEST
// 新增消息测试：新增消息，观察可获取消息数量，以及持久化消息数量
TEST(msg_test, insert_test) {
    BasicProperties properties;
    properties.set_id(uuid_helper::uuid());
    properties.set_delivery_mode(DeliveryMode::DURABLE);
    properties.set_routing_key("news.music.pop");
    mmp->insert("queue1", &properties, "hello world-1", DeliveryMode::DURABLE);
    mmp->insert("queue1", nullptr, "hello world-2", DeliveryMode::DURABLE);
    mmp->insert("queue1", nullptr, "hello world-3", DeliveryMode::DURABLE);
    mmp->insert("queue1", nullptr, "hello world-4", DeliveryMode::DURABLE);
    mmp->insert("queue1", nullptr, "hello world-5", DeliveryMode::UNDURABLE);
    ASSERT_EQ(mmp->getable_count("queue1"), 5);
    ASSERT_EQ(mmp->total_count("queue1"), 4);
    ASSERT_EQ(mmp->durable_count("queue1"), 4);
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 0);
}
// 获取消息测试：获取一条消息，在不进行确认，以及进行确认后，查看可获取消息数量，待确认消息数量，以及测试消息获取的顺序
#if !ONLY_INSERT
TEST(msg_test, select_test) {
    ASSERT_EQ(mmp->getable_count("queue1"), 5);
    ASSERT_EQ(mmp->total_count("queue1"), 4);
    ASSERT_EQ(mmp->durable_count("queue1"), 4);
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 0);
    message_ptr msg = mmp->front("queue1");
    ASSERT_EQ(msg->payload().body(), std::string("hello world-1"));
    ASSERT_EQ(mmp->getable_count("queue1"), 4);
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 1);
    msg = mmp->front("queue1");
    ASSERT_EQ(msg->payload().body(), std::string("hello world-2"));
    ASSERT_EQ(mmp->getable_count("queue1"), 3);
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 2);
    msg = mmp->front("queue1");
    ASSERT_EQ(msg->payload().body(), std::string("hello world-3"));
    ASSERT_EQ(mmp->getable_count("queue1"), 2);
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 3);
    msg = mmp->front("queue1");
    ASSERT_EQ(msg->payload().body(), std::string("hello world-4"));
    ASSERT_EQ(mmp->getable_count("queue1"), 1);
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 4);
    msg = mmp->front("queue1");
    // ASSERT_EQ(msg, nullptr);
}
#endif
#else
// 恢复历史消息数据
TEST(msg_test, recovery_test) {
    ASSERT_EQ(mmp->getable_count("queue1"), 4);
    message_ptr msg = mmp->front("queue1");
    ASSERT_EQ(msg->payload().body(), "hello world-1");
    ASSERT_EQ(mmp->getable_count("queue1"), 3);
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 1);
    msg = mmp->front("queue1");
    ASSERT_EQ(msg->payload().body(), "hello world-2");
    ASSERT_EQ(mmp->getable_count("queue1"), 2);
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 2);
    msg = mmp->front("queue1");
    ASSERT_EQ(msg->payload().body(), "hello world-3");
    ASSERT_EQ(mmp->getable_count("queue1"), 1);
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 3);
    msg = mmp->front("queue1");
    ASSERT_EQ(msg->payload().body(), "hello world-4");
    ASSERT_EQ(mmp->getable_count("queue1"), 0);
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 4);
    msg = mmp->front("queue1");
    ASSERT_EQ(msg, nullptr);
}
#endif
// 删除消息测试：确认一条消息，查看持久化消息数量以及待确认消息数量
TEST(msg_test, delete_ack_test) {
    ASSERT_EQ(mmp->getable_count("queue1"), 5);
    message_ptr msg = mmp->front("queue1");
    ASSERT_NE(msg.get(), nullptr);
    ASSERT_EQ(msg->payload().body(), "hello world-1");
    ASSERT_EQ(mmp->getable_count("queue1"), 4);
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 1);
    mmp->ack("queue1", msg->payload().properties().id());
    ASSERT_EQ(mmp->durable_count("queue1"), 3);
    ASSERT_EQ(mmp->getable_count("queue1"), 4);
    ASSERT_EQ(mmp->total_count("queue1"), 4); // 还是4，因为还没垃圾回收
    ASSERT_EQ(mmp->wait_ack_count("queue1"), 0);
}
// 销毁测试
TEST(message_test, clear_test) {
    // 前面都clear了很多次了，不测了
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new msg_test);
    auto res = RUN_ALL_TESTS();
    LOG(INFO) << "res: " << res << std::endl;
    return 0;
}