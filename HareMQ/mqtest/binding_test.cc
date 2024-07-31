/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqserver/binding.hpp"
#include <gtest/gtest.h>

using namespace hare_mq;


binding_manager::ptr bmptr = nullptr;

class binding_test : public testing::Environment {
public:
    virtual void SetUp() override {
        bmptr = std::make_shared<binding_manager>("./data-binding/meta.db");
    }
    virtual void TearDown() override {
        // bmptr->clear_bindings(); //
    }
};


#define RECOVERY_TEST 1 // 是否测试recovery
#if !RECOVERY_TEST
TEST(binding_test, insert_test) {
    bmptr->bind("exchange1", "queue1", "news.music.#", true);
    bmptr->bind("exchange1", "queue2", "news.sport.#", true);
    bmptr->bind("exchange1", "queue3", "news.sport.basketball", true);
    bmptr->bind("exchange2", "queue1", "news.music.pop", true);
    bmptr->bind("exchange2", "queue2", "news.sport.football", true);
    bmptr->bind("exchange2", "queue3", "news.sport.swimming", true);
    /**
     * e1, q1
     * e1, q2
     * e1, q3
     * e2, q1
     * e2, q2
     * e2, q3
     */
    ASSERT_EQ(bmptr->size(), 6);
}
TEST(binding_test, select_test) {
    /**
     * e1, q1
     * e1, q2
     * e1, q3
     * e2, q1
     * e2, q2
     * e2, q3
     */
    ASSERT_EQ(bmptr->exists("exchange1", "queue1"), true);
    ASSERT_EQ(bmptr->exists("exchange1", "queue2"), true);
    ASSERT_EQ(bmptr->exists("exchange1", "queue3"), true);
    ASSERT_EQ(bmptr->exists("exchange2", "queue1"), true);
    ASSERT_EQ(bmptr->exists("exchange2", "queue2"), true);
    ASSERT_EQ(bmptr->exists("exchange2", "queue3"), true);

    binding::ptr bp = bmptr->get_binding("exchange1", "queue1");
    ASSERT_NE(bp, nullptr);
    ASSERT_EQ(bp->exchange_name, std::string("exchange1"));
    ASSERT_EQ(bp->msg_queue_name, std::string("queue1"));
    ASSERT_EQ(bp->binding_key, std::string("news.music.#"));
}

TEST(binding_test, select_exchange_test) {
    /**
     * e1, q1
     * e1, q2
     * e1, q3
     * e2, q1
     * e2, q2
     * e2, q3
     */
    msg_queue_binding_map mqbm = bmptr->get_exchange_bindings("exchange1");
    ASSERT_EQ(mqbm.size(), 3);
    ASSERT_NE(mqbm.find("queue1"), mqbm.end());
    ASSERT_NE(mqbm.find("queue2"), mqbm.end());
    ASSERT_NE(mqbm.find("queue3"), mqbm.end());
}

TEST(binding_test, remove_queue_test) {
    /**
     * e1, q1
     * e1, q2
     * e1, q3
     * e2, q1
     * e2, q2
     * e2, q3
     */
    bmptr->unbind_queue("queue1");
    ASSERT_EQ(bmptr->exists("exchange1", "queue1"), false);
    ASSERT_EQ(bmptr->exists("exchange2", "queue1"), false);
    /**
     * e1, q2
     * e1, q3
     * e2, q2
     * e2, q3
     */
}

TEST(binding, remove_exchange_test) {
    /**
     * e1, q2
     * e1, q3
     * e2, q2
     * e2, q3
     */
    bmptr->unbind_exchange("exchange1");
    ASSERT_EQ(bmptr->exists("exchange1", "queue1"), false);
    ASSERT_EQ(bmptr->exists("exchange1", "queue2"), false);
    ASSERT_EQ(bmptr->exists("exchange1", "queue3"), false);
    /**
     * e2, q2
     * e2, q3
     */
}

TEST(binding, remove_test) {
    /**
     * e2, q2
     * e2, q3
     */
    bmptr->unbind("exchange2", "queue3");
    ASSERT_EQ(bmptr->exists("exchange2", "queue3"), false); // 刚刚删掉的
    ASSERT_EQ(bmptr->exists("exchange2", "queue1"), false); // remove_queue_test 里删掉了
    ASSERT_EQ(bmptr->exists("exchange2", "queue2"), true); // 还在
    /**
     * e2, q2
     */
}
#else
TEST(binding, recovery_test) {
    /**
     * e2, q2
     */
    ASSERT_EQ(bmptr->size(), 1);
    ASSERT_EQ(bmptr->exists("exchange1", "queue1"), false);
    ASSERT_EQ(bmptr->exists("exchange1", "queue2"), false);
    ASSERT_EQ(bmptr->exists("exchange1", "queue3"), false);
    ASSERT_EQ(bmptr->exists("exchange2", "queue1"), false);
    ASSERT_EQ(bmptr->exists("exchange2", "queue2"), true);
    ASSERT_EQ(bmptr->exists("exchange2", "queue3"), false);
}
#endif

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new binding_test);
    auto res = RUN_ALL_TESTS();
    LOG(INFO) << "res: " << res << std::endl;
    return 0;
}