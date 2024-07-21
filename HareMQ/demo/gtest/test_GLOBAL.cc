/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../log.hpp"
#include <gtest/gtest.h>
#include <iostream>

class MyEnv : public testing::Environment {
public:
    virtual void SetUp() override {
        LOG(INFO) << "before testing, init ..." << std::endl;
    }
    virtual void TearDown() override {
        LOG(INFO) << "after testing, clearing ..." << std::endl;
    }
};
TEST(MyEnv, test1) {
    LOG(INFO) << "unit test1" << std::endl;
}
TEST(MyEnv, test2) {
    LOG(INFO) << "unit test2" << std::endl;
}
int main(int argc, char** argv) {
    testing::AddGlobalTestEnvironment(new MyEnv);
    testing::InitGoogleTest(&argc, argv);
    auto res = RUN_ALL_TESTS();
    LOG(INFO) << "res: " << res << std::endl;
    return 0;
}