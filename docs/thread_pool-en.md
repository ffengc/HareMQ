# Implementing a thread pool using C++11 asynchronous operations

- [简体中文](./thread_pool.md)
- [English](./thread_pool-en.md)

***
- [Implementing a thread pool using C++11 asynchronous operations](#implementing-a-thread-pool-using-c11-asynchronous-operations)
  - [Introduction](#introduction)
  - [Explanation of some code details](#explanation-of-some-code-details)
  - [test](#test)


## Introduction

When executing tasks based on the thread pool, the internal execution logic of the entry function is fixed, so the combination of `std::packaged_task` and `std::future` is selected to implement it.

Working idea of ​​thread pool:
- The user passes in the function to be executed and the data to be processed (function parameters), and the working thread in the thread pool executes the function to complete the task;

**Code: [HareMQ/demo/thread_pool](../HareMQ/demo/thread_pool/thread_pool.hpp)**


## Explanation of some code details

**Explanation of `push` function parameters**

```cpp
    template <typename func, typename... Args>
    auto push(func&& f, Args&&... args) -> std::future<decltype(f(args...))>;
```
The first thing pushed in is a function (the function that the user wants to execute), followed by an indefinite parameter, which represents the data to be processed, that is, the parameter to be inserted. Then, inside push, this function is packaged into an asynchronous operation (packaged_task) and thrown to the thread for execution!

**Explanation of return value**

Because we don't know what type of value the user's thread returns to us (because we don't know what the function the user wants to execute will look like), we can only use `auto` to indicate the type, but using `auto` directly will definitely not work, because the system doesn't know how much space to open up to push onto the stack, so C++ provides a type deduction: `decltype(func(args...))` indicates that the return type is the return type of `func(args...)`.

**Some explanation inside the `push` function**

```cpp
    auto push(func&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        // 1. Package the passed function into a packaged_task task package
        using return_type = decltype(f(args...)); // return type
        auto bind_func = std::bind(std::forward<func>(f), std::forward<Args>(args)...); // Function + parameter type
        auto task = std::make_shared<std::packaged_task<return_type()>>(bind_func);
        std::future<return_type> fu = task->get_future();
        // 2. Construct a lambda anonymous function (capture the task object and execute the task object within the function)
        {
            std::unique_lock<std::mutex> lock(__mtx_lock);
            // 3. Throw the constructed anonymous function object into the task queue
            __task_queue.push_back([task]() {
                (*task)();
            });
            // 4. Awakening consumers
            __cond.notify_one();
        }
        return fu;
    }
```

First, because the functions passed in by each user may be different, the parameters and return values ​​are different, so we must unify them in `push`.

First, the return value type, we need to deduce:

```cpp
using return_type = decltype(f(args...)); // return type
```

Then we need to bind the function and function parameters together, so we need to use `bind`. Because it is a variable parameter, we need to expand it with `...`. In order to maintain the properties of the parameters, we need to use a perfect forwarding `std::forward`.

```cpp
auto bind_func = std::bind(std::forward<func>(f), std::forward<Args>(args)...); // Function + parameter type
```

Then the rest is the same as the demo in [asynchronous.md](./asynchronous.md) , using a pointer to encapsulate `std::packaged_task`.


```cpp
auto task = std::make_shared<std::packaged_task<return_type()>>(bind_func);
std::future<return_type> fu = task->get_future();
```

## test

```cpp
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
```

![](./assets/23.png)