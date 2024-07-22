/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#include "../log.hpp"
#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <thread>

/**
 * packaged_task 的使用
 * packaged_task 是一个模版类，实例化的对象可以对一个函数进行二次封装
 * packaged_task 可以通过 get_future 获取一个 future 对象，来获取封装的这个函数的异步执行结果
 */

int add(int num1, int num2) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG(INFO) << "called: add(int, int)" << std::endl;
    return num1 + num2;
}

int main() {
    // std::packaged_task<int(int, int)> task(add);
    // std::future<int> fu = task.get_future();

    // task(11, 22) 可以这样调用
    // 但是不能当作函数对象，也就是说，不能这样调用 std::thread thr(task) 这样是不行的
    // std::thread thr(task) 和 std::async(std::launch::async, task) 本质是一样的，都是异步操作，async里面也是起线程
    // std::async(std::launch::async, task, 11, 22); // 不能这样调用

    // 但是我们可以把task定义成一个指针，传递到线程中，然后进行解引用执行
    // 如果用普通指针，容易出现指针的生命周期问题，所以使用智能指针

    auto ptask = std::make_shared<std::packaged_task<int(int, int)>>(add);
    std::thread thr([ptask]() {
        (*ptask)(11, 12); // 调用这个函数
    });
    std::future<int> fu = ptask->get_future();
    LOG(INFO) << "result: " << fu.get() << std::endl;
    thr.join();
    return 0;
}