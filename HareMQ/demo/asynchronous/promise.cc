/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../log.hpp"
#include <future>
#include <iostream>
#include <thread>
#include <chrono>

int add(int num1, int num2, std::promise<int>& prom) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG(INFO) << "called: add(int, int)" << std::endl;
    prom.set_value(num1 + num2);
    return num1 + num2;
}

// int add(int num1, int num2, int* result) {
//     std::this_thread::sleep_for(std::chrono::seconds(3));
//     LOG(INFO) << "called: add(int, int)" << std::endl;
//     *result = num1 + num2;
//     return num1 + num2;
// }


int main() {
    // 通过 get_future 来建立 prom和fu 的关联
    std::promise<int> prom;
    std::future<int> fu = prom.get_future();
    // int result = 0;
    std::thread thr(add, 11, 22, std::ref(prom));
    int res = fu.get();
    LOG(INFO) << "sum: " << res << std::endl;
    thr.join();
    return 0;
}