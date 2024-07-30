/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqserver/queue.hpp"
#include <gtest/gtest.h>

using namespace hare_mq;

msg_queue_manager::ptr qmptr = nullptr;

#define IF_GTEST 1

#if IF_GTEST
class queue_test : public testing::Environment {
public:
    virtual void SetUp() override {
        qmptr = std::make_shared<msg_queue_manager>("./data-queue/meta.db");
    }
    virtual void TearDown() override {
        qmptr->clear_queues(); //
    }
};

TEST(queue_test, insert_test) {
    std::unordered_map<std::string, std::string> map = { { "k1", "v1" } };
    qmptr->declare_queue("queue1", true, true, false, map);
    qmptr->declare_queue("queue2", true, true, false, map);
    qmptr->declare_queue("queue3", true, true, false, map);
    qmptr->declare_queue("queue4", true, true, false, map);
    ASSERT_EQ(qmptr->size(), 4);
}
TEST(queue_test, select_test) {
    msg_queue::ptr qptr = qmptr->select_queue("queue4");
    ASSERT_NE(qptr, nullptr);
    ASSERT_EQ(qptr->name, "queue4");
    ASSERT_EQ(qptr->durable, true);
    ASSERT_EQ(qptr->exclusive, true);
    ASSERT_EQ(qptr->auto_delete, false);
    ASSERT_EQ(qptr->get_args(), "k1=v1&");
}
TEST(queue_test, remove_test) {
    qmptr->delete_queue("queue3");
    msg_queue::ptr qptr = qmptr->select_queue("queue3");
    ASSERT_EQ(qptr, nullptr); // 这里应该是找不到的
    ASSERT_EQ(qmptr->exists("queue3"), false); // 这里应该是找不到的
}
TEST(queue_test, recovery) {
    // 测试加载历史数据
    // 跑一次不clear的, 然后屏蔽insert的测试，然后来测试recovery
    ASSERT_EQ(qmptr->exists("queue1"), true);
    ASSERT_EQ(qmptr->exists("queue2"), true);
    ASSERT_EQ(qmptr->exists("queue3"), false); // 这里应该是找不到的, 在上面被删掉了
    ASSERT_EQ(qmptr->exists("queue4"), true);
}
#endif

#if !IF_GTEST
void my_test() {
}
#endif

int main(int argc, char** argv) {
#if IF_GTEST
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new queue_test);
    auto res = RUN_ALL_TESTS();
    LOG(INFO) << "res: " << res << std::endl;
#else
    my_test();
#endif
    return 0;
}