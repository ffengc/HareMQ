# Asynchronous operations in C++11

- [简体中文](./asynchronous.md)
- [English](./asynchronous-en.md)

***
- [Asynchronous operations in C++11](#asynchronous-operations-in-c11)
  - [What is `std::future`](#what-is-stdfuture)
  - [Use `std::async` to associate asynchronous tasks](#use-stdasync-to-associate-asynchronous-tasks)
  - [Example](#example)
  - [Use `std::promise` and `std::future` together](#use-stdpromise-and-stdfuture-together)
  - [Using `std::packaged_task` with `std::future`](#using-stdpackaged_task-with-stdfuture)


The main purpose of learning this part is to implement a thread pool. If a thread pool does not care about the running results of the threads, and only needs to throw tasks out, then this thread pool is very simple and easy to implement. However, if we care about the results of the thread pool, asynchronous operations are required to implement it.

## What is `std::future`

**Documentation: [https://legacy.cplusplus.com/reference/future](https://legacy.cplusplus.com/reference/future/future/?kw=future)**

`std::future` is a template class in the C++11 standard library that represents the result of an asynchronous operation. When we use asynchronous tasks in multithreaded programming, `std:future` can help us get the execution results of the task when we need it. An important feature of `std:future` is that it can block the current thread until the asynchronous operation is completed, thus ensuring that we will not encounter unfinished operations when getting the results.

**Application scenarios**

- Asynchronous tasks: When we need to perform some time-consuming operations in the background, such as network requests or computationally intensive tasks, `std:future` can be used to represent the results of these asynchronous tasks. By separating tasks from the main thread, we can achieve parallel processing of tasks, thereby improving the execution efficiency of the program.
- Concurrency control: In multi-threaded programming, we may need to wait for certain tasks to complete before continuing to perform other operations. By using `std:future`, we can synchronize threads to ensure that the results are obtained after the task is completed and continue to perform subsequent operations
- Result acquisition: `std:future` provides a safe way to obtain the results of asynchronous tasks. We can use the `std::future:get()` function to obtain the results of the task, which blocks the current thread until the asynchronous operation is completed. In this way, when calling the `get()` function, we can ensure that the required results have been obtained.


## Use `std::async` to associate asynchronous tasks

`std::async` is a simple method that associates a task with a `std::future` task and returns a task associated with it. It creates and runs a `std::future` object associated with the output task result. By default, `std:async` starts a new thread or waits for the parameter in `future`. When this parameter is of `std::launch` type, whether the task runs synchronously depends on the parameter you give, which is of `std::launch` type.
- `std::launch::deferred` indicates that the coefficient will be called delayed until `get()` or `wait()` is called on `future`.
- `std::launch::async` indicates that the function will run on the thread created by itself.
- `std::launch::deferred`, `std::launch::async` automatically selects the strategy based on system and other conditions.


## Example

```cpp
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
    std::future<int> res = std::async(std::launch::deferred, add, 11, 12);
    LOG(INFO) << "get res: " << res.get() << std::endl;
    return 0;
}
```

![](./assets/19.png)

It is obviously asynchronous.

Obviously, if we don't call `get()`, it will not be executed.

If it is replaced with `std::future<int> res = std::async(std::launch::async, add, 11, 12);`, the effect will be different. As long as `async` is called, it will start to execute. When it will be executed, it is unknown! But it will definitely not wait until `get()` is called.

## Use `std::promise` and `std::future` together

**Document: [https://legacy.cplusplus.com/reference/future/promise/](https://legacy.cplusplus.com/reference/future/promise/?kw=promise)**

`std::promise` provides a way to set a value, which can be read through the associated `std::future` object after setting. In other words, as mentioned before, `std::future` can read the return value of an asynchronous function, but it has to wait for it to be ready, and `std::promise` provides a way to manually make `std:future` ready.


```cpp
#include "../log.hpp"
#include <future>
#include <iostream>
#include <thread>

int add(int num1, int num2, std::promise<int>& prom) {
    LOG(INFO) << "called: add(int, int)" << std::endl;
    prom.set_value(num1 + num2);
    return num1 + num2;
}

int main() {
    // Use get_future to establish the association between prom and fu
    std::promise<int> prom;
    std::future<int> fu = prom.get_future();
    std::thread thr(add, 11, 22, std::ref(prom));
    int res = fu.get();
    LOG(INFO) << "sum: " << res << std::endl;
    thr.join();
    return 0;
}
```

It is unknown when the task executed by the thread will be executed, but we can use `get_future` to obtain the amount in the asynchronous task.

![](./assets/20.png)

> [!NOTE]
> Why do we take this approach? Can't we get the value by directly passing an integer address? If we do this, because the thread execution is asynchronous, we don't know when it will be set. If we use `promise`, we can ensure that the asynchronous result must be assigned before it can be obtained outside. This is a synchronous control. Other methods do not have synchronous control! For example, the following code uses output parameters to set.

```cpp
int add(int num1, int num2, int* result) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG(INFO) << "called: add(int, int)" << std::endl;
    *result = num1 + num2;
    return num1 + num2;
}
int main() {
    int result = 0;
    std::thread thr(add, 11, 22, &result);
    int res = result;
    LOG(INFO) << "sum: " << res << std::endl;
    thr.join();
    return 0;
}
```

![](./assets/21.png)

There is no synchronization, so the result obtained from the outside will not wait for the calculation to end, but will directly get the result that has not been set. In essence, there is no synchronization! In fact, there are other ways to synchronize, such as the synchronization variables in the `pthread` library, which is the same principle.

## Using `std::packaged_task` with `std::future`

**Documentation: [https://legacy.cplusplus.com/reference/future/packaged_task/](https://legacy.cplusplus.com/reference/future/packaged_task/)**

```cpp
int add(int num1, int num2) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    LOG(INFO) << "called: add(int, int)" << std::endl;
    return num1 + num2;
}

int main() {
    // std::packaged_task<int(int, int)> task(add);
    // std::future<int> fu = task.get_future();

    // task(11, 22) can be called like this
    // But it cannot be used as a function object, that is, it cannot be called like this std::thread thr(task) This is not allowed
    // std::thread thr(task) and std::async(std::launch::async, task) are essentially the same, both are asynchronous operations, and async also starts a thread
    // std::async(std::launch::async, task, 11, 22); // Cannot be called like this

    // But we can define task as a pointer, pass it to the thread, and then dereference it for execution
    // If you use ordinary pointers, it is easy to have pointer life cycle problems, so use smart pointers

    auto ptask = std::make_shared<std::packaged_task<int(int, int)>>(add);
    std::thread thr([ptask]() {
        (*ptask); // call this function
    });
    std::future<int> fu = ptask->get_future();
    LOG(INFO) << "result: " << fu.get() << std::endl;
    thr.join();
    return 0;
}
```

![](./assets/22.png)