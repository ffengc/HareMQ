/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqserver/consumer.hpp"
#include <gtest/gtest.h>

using namespace hare_mq;

consumer_manager::ptr cmp = nullptr;

class consumer_test : public testing::Environment {
public:
    virtual void SetUp() override {
        cmp = std::make_shared<consumer_manager>();
        cmp->init_queue_consumer("queue1");
    }
    virtual void TearDown() override {
        // cmp->clear();
    }
};

void cb(const std::string& tag, const BasicProperties* bp, const std::string& body) {
    std::cout << tag << " consume a mesg: " << body << std::endl;
}

// 测试新增
TEST(consumer_test, insert_test) {
    cmp->create("consumer1", "queue1", false, cb);
    cmp->create("consumer2", "queue1", false, cb);
    cmp->create("consumer3", "queue1", false, cb);
    ASSERT_EQ(cmp->exists("consumer1", "queue1"), true);
    ASSERT_EQ(cmp->exists("consumer2", "queue1"), true);
    ASSERT_EQ(cmp->exists("consumer3", "queue1"), true);
    ASSERT_EQ(cmp->exists("consumer3", "queue2"), false);
    ASSERT_EQ(cmp->exists("consumer4", "queue1"), false);
}
TEST(consumer_test, remove_test) {
    cmp->remove("consumer1", "queue1");
    ASSERT_EQ(cmp->exists("consumer1", "queue1"), false);
    ASSERT_EQ(cmp->exists("consumer2", "queue1"), true);
    ASSERT_EQ(cmp->exists("consumer3", "queue1"), true);
}

TEST(consumer_test, choose_test) {
    consumer::ptr cp = cmp->choose("queue1");
    ASSERT_NE(cp, nullptr);
    ASSERT_EQ(cp->tag, "consumer2");

    cp = cmp->choose("queue1");
    ASSERT_NE(cp, nullptr);
    ASSERT_EQ(cp->tag, "consumer3");

    cp = cmp->choose("queue1");
    ASSERT_NE(cp, nullptr);
    ASSERT_EQ(cp->tag, "consumer2");
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new consumer_test);
    auto res = RUN_ALL_TESTS();
    LOG(INFO) << "res: " << res << std::endl;
    return 0;
}