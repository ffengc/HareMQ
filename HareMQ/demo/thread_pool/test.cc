/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "thread_pool.hpp"

int add(int a, int b) { return a + b; }

int main() {
    thread_pool pool;
    for (int i = 0; i < 11; i++) {
        std::future<int> fu = pool.push(add, i, 22);
        LOG(INFO) << "add res: " << fu.get() << std::endl;
    }
    pool.stop();
    return 0;
}