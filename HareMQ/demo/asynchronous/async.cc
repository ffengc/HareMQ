/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../log.hpp"
#include <future>
#include <iostream>
#include <thread>

int add(int num1, int num2) {
    LOG(INFO) << "called: add(int, int)" << std::endl;
    return num1 + num2;
}
int main() {
    // std::async(func, ...), std::async(policy, func, ...)
    std::future<int> res = std::async(std::launch::async, add, 11, 12);
    // std::future<int> res = std::async(std::launch::deferred, add, 11, 12);
    LOG(INFO) << "get res: " << res.get() << std::endl;
    return 0;
}