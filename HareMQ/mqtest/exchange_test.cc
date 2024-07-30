/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../mqserver/exchange.hpp"
#include <gtest/gtest.h>

using namespace hare_mq;

exchange_manager::ptr emp = nullptr;

#define IF_GTEST 1

#if IF_GTEST
class exchange_test : public testing::Environment {
public:
    virtual void SetUp() override {
        emp = std::make_shared<exchange_manager>("./data-exchange/meta.db");
    }
    virtual void TearDown() override {
        // emp->clear_exchange(); //
    }
};

TEST(exchange_test, insert_test) {
    std::unordered_map<std::string, std::string> map = { { "k1", "v1" }, { "k2", "v2" } };
    emp->declare_exchange("exchange1", ExchangeType::DIRECT, true, false, map);
    emp->declare_exchange("exchange2", ExchangeType::DIRECT, true, false, map);
    emp->declare_exchange("exchange3", ExchangeType::DIRECT, true, false, map);
    emp->declare_exchange("exchange4", ExchangeType::DIRECT, true, false, map);
    ASSERT_EQ(emp->size(), 4);
}
TEST(exchange_test, select_test) {
    exchange::ptr exp = emp->select_exchange("exchange3");
    ASSERT_NE(exp, nullptr);
    ASSERT_EQ(exp->name, "exchange3");
    ASSERT_EQ(exp->durable, true);
    ASSERT_EQ(exp->auto_delete, false);
    ASSERT_EQ(exp->type, ExchangeType::DIRECT);
    // ASSERT_EQ(exp->get_args(), std::string("k1=v1&k2=v2&"));
}
TEST(exchange_test, remove_test) {
    emp->delete_exchange("exchange3");
    exchange::ptr exp = emp->select_exchange("exchange3");
    ASSERT_EQ(exp, nullptr); // 这里应该是找不到的
    ASSERT_EQ(emp->exists("exchange3"), false); // 这里应该是找不到的
}
TEST(exchange_test, recovery) {
    // 测试加载历史数据
    // 跑一次不clear的, 然后屏蔽insert的测试，然后来测试recovery
    ASSERT_EQ(emp->exists("exchange1"), true);
    ASSERT_EQ(emp->exists("exchange2"), true);
    ASSERT_EQ(emp->exists("exchange3"), false); // 这里应该是找不到的, 在上面被删掉了
    ASSERT_EQ(emp->exists("exchange4"), true);
}
#endif


#if !IF_GTEST
void my_test() {
    emp = std::make_shared<exchange_manager>("./data/meta.db");
    std::unordered_map<std::string, std::string> map = { { "k1", "v1" }, { "k2", "v2" } };
    emp->declare_exchange("exchange1", ExchangeType::DIRECT, true, false, map);
    emp->declare_exchange("exchange2", ExchangeType::DIRECT, true, false, map);
    emp->declare_exchange("exchange3", ExchangeType::DIRECT, true, false, map);
    emp->declare_exchange("exchange4", ExchangeType::DIRECT, true, false, map);
}
#endif

int main(int argc, char** argv) {
#if IF_GTEST
    testing::InitGoogleTest(&argc, argv);
    testing::AddGlobalTestEnvironment(new exchange_test);
    auto res = RUN_ALL_TESTS();
    LOG(INFO) << "res: " << res << std::endl;
#else
    my_test();
#endif
    return 0;
}