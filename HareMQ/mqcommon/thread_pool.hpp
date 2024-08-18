/*
 * Write by Yufc
 * See https://github.com/ffengc/HareMQ
 * please cite my project link: https://github.com/ffengc/HareMQ when you use this code
 */

#ifndef __YUFC_THREAD_POOL__
#define __YUFC_THREAD_POOL__

#include "../mqcommon/logger.hpp"
#include <assert.h>
#include <condition_variable>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

class thread_pool {
public:
    using func_t = std::function<void(void)>;
    using ptr = std::shared_ptr<thread_pool>; //
private:
    std::atomic<bool> __stop_signal; // 线程池停止信号
    std::mutex __mtx_lock; // 互斥锁
    std::condition_variable __cond; // 同步变量
    std::vector<std::thread> __threads; // 线程池的线程
    std::deque<func_t> __task_queue; // 任务队列

public:
    thread_pool(int thread_count = 1)
        : __stop_signal(false) {
        // 创建线程
        for (int i = 0; i < thread_count; i++) {
            __threads.emplace_back(&thread_pool::entry, this);
        }
    }
    ~thread_pool() {
        stop();
    }
    void stop() {
        if (__stop_signal == true)
            return; // 如果已经退出了就不能重复退出了
        __stop_signal = true;
        __cond.notify_all();
        for (auto& e : __threads) // 等待所有线程退出
            e.join();
    }

    template <typename func, typename... Args>
    auto push(func&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        // 1. 对传入的函数封装成 packaged_task 任务包
        using return_type = decltype(f(args...)); // 返回值类型
        auto bind_func = std::bind(std::forward<func>(f), std::forward<Args>(args)...); // 函数+参数类型
        auto task = std::make_shared<std::packaged_task<return_type()>>(bind_func);
        std::future<return_type> fu = task->get_future();
        // 2. 构造 lambda 匿名函数（捕获任务对象，函数内执行任务对象）
        {
            std::unique_lock<std::mutex> lock(__mtx_lock);
            // 3. 将构造出来的匿名函数对象，抛入到任务队列中
            __task_queue.push_back([task]() {
                (*task)();
            });
            // 4. 唤醒消费者
            __cond.notify_one();
        }
        return fu;
    }

private:
    // 线程入口函数 -- 不断从任务队列中取出任务进行执行
    void entry() {
        while (!__stop_signal) {
            std::deque<func_t> tmp;
            {
                // 1. 加锁
                std::unique_lock<std::mutex> lock(__mtx_lock);
                // 2. 任务队列不为空，或者 __stop_signal 被置位, 否则就一直等着
                __cond.wait(lock, [this]() { return __stop_signal || !__task_queue.empty(); });
                // 3. 因为现在是加锁状态，一次取一个太亏了，如果就绪多个，可以一次取出来的}
                tmp.swap(__task_queue);
                assert(__task_queue.size() == 0); // 此时任务队列应该为空了
            }
            for (auto& t : tmp) // 逐个执行即可(无锁状态)
                t();
        }
    }
};
#endif