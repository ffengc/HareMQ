/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 * g++ test.cc -o gtest -lgtest
 */
#include <gtest/gtest.h>
int add(int a, int b) {
    return a + b;
}
TEST(testCase, test1) {
    EXPECT_EQ(add(2, 3), 5);
}
int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}