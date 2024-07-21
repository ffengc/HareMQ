/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */


#include "../log.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <unordered_map>

class MyTest : public testing::Test {
public:
    virtual void SetUp() override { }
    virtual void TearDown() override { }
    static void SetUpTestCase() {
        LOG(INFO) << "before each suit test, init ..." << std::endl;
    }
    static void TearDownTestCase() {
        LOG(INFO) << "after each suit test, clearing ..." << std::endl;
    }

public:
    std::unordered_map<std::string, std::string> __map;
};

TEST_F(MyTest, insert_test) {
    __map.insert({ "hello", "nihao" });
    __map.insert({ "bye", "zaijian" });
}
TEST_F(MyTest, size_test) {
    ASSERT_EQ(__map.size(), 2);
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    auto res = RUN_ALL_TESTS();
    LOG(INFO) << "res: " << res << std::endl;
    return 0;
}