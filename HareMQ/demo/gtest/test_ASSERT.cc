/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../log.hpp"
#include <gtest/gtest.h>
#include <iostream>

/**
 * 断言宏的使用:
 *      ASSERT_ 断言失败则推出
 *      EXPECT_ 断言失败则继续运行
 * 注意:
 *      断言宏，必须在单元测试宏函数中使用
 */

TEST(test1, MYTEST) {
    int age = 20;
    ASSERT_GT(age, 18);
    std::cout << "OK" << std::endl;
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    auto res = RUN_ALL_TESTS();
    LOG(INFO) << "res: " << res << std::endl;
    return 0;
}