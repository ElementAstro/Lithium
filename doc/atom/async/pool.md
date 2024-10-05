# Documentation for pool.hpp

This document provides a detailed overview of the `pool.hpp` header file, which contains implementations for a thread-safe queue and a thread pool in C++.

## Table of Contents

1. [ThreadSafeQueue Class](#threadsafequeue-class)
2. [ThreadPool Class](#threadpool-class)
3. [Usage Examples](#usage-examples)

## ThreadSafeQueue Class

The `ThreadSafeQueue` class is a thread-safe implementation of a double-ended queue (deque).

### Template Parameters

- `T`: The type of elements stored in the queue.
- `Lock`: The type of lock to use (default is `std::mutex`).

### Public Methods

#### Constructors and Assignment Operators

```cpp
ThreadSafeQueue();
ThreadSafeQueue(const ThreadSafeQueue& other);
ThreadSafeQueue& operator=(const ThreadSafeQueue& other);
ThreadSafeQueue(ThreadSafeQueue&& other) noexcept;
ThreadSafeQueue& operator=(ThreadSafeQueue&& other) noexcept;
```

#### Element Access and Modification

```cpp
void pushBack(T&& value);
void pushFront(T&& value);
std::optional<T> popFront();
std::optional<T> popBack();
std::optional<T> steal();
void rotateToFront(const T& item);
std::optional<T> copyFrontAndRotateToBack();
void clear();
```

#### Capacity

```cpp
bool empty() const;
size_type size() const;
```

## ThreadPool Class

The `ThreadPool` class provides a simple thread pool implementation for executing tasks asynchronously.

### Template Parameters

- `FunctionType`: The type of function to be executed (default is `std::move_only_function<void()>` if available, otherwise `std::function<void()>`).
- `ThreadType`: The type of thread to use (default is `std::jthread`).

### Public Methods

#### Constructor

```cpp
explicit ThreadPool(
    const unsigned int& number_of_threads = std::thread::hardware_concurrency(),
    InitializationFunction init = [](std::size_t) {});
```

Creates a thread pool with the specified number of threads and an optional initialization function for each thread.

#### Task Enqueuing

```cpp
template <typename Function, typename... Args,
          typename ReturnType = std::invoke_result_t<Function&&, Args&&...>>
std::future<ReturnType> enqueue(Function func, Args... args);

template <typename Function, typename... Args>
void enqueueDetach(Function&& func, Args&&... args);
```

Enqueue tasks to be executed by the thread pool. `enqueue` returns a `std::future` for the task result, while `enqueueDetach` doesn't return anything.

#### Utility Methods

```cpp
std::size_t size() const;
void waitForTasks();
```

`size()` returns the number of threads in the pool, and `waitForTasks()` blocks until all tasks are completed.

## Usage Examples

Here are some examples demonstrating how to use the `ThreadSafeQueue` and `ThreadPool` classes:

### ThreadSafeQueue Example

```cpp
#include "pool.hpp"
#include <iostream>

int main() {
    atom::async::ThreadSafeQueue<int> queue;

    // Push elements
    queue.pushBack(1);
    queue.pushBack(2);
    queue.pushFront(0);

    // Pop elements
    auto front = queue.popFront();
    if (front) {
        std::cout << "Front element: " << *front << std::endl;
    }

    // Check size
    std::cout << "Queue size: " << queue.size() << std::endl;

    // Clear the queue
    queue.clear();

    std::cout << "Is queue empty? " << (queue.empty() ? "Yes" : "No") << std::endl;

    return 0;
}
```

### ThreadPool Example

```cpp
#include "pool.hpp"
#include <iostream>
#include <vector>

int main() {
    atom::async::ThreadPool pool(4); // Create a thread pool with 4 threads

    std::vector<std::future<int>> results;

    // Enqueue tasks
    for (int i = 0; i < 10; ++i) {
        results.push_back(pool.enqueue([i] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return i * i;
        }));
    }

    // Get results
    for (auto& result : results) {
        std::cout << "Result: " << result.get() << std::endl;
    }

    // Enqueue a task without returning a result
    pool.enqueueDetach([] {
        std::cout << "Detached task executed" << std::endl;
    });

    // Wait for all tasks to complete
    pool.waitForTasks();

    return 0;
}
```

These examples demonstrate basic usage of the `ThreadSafeQueue` and `ThreadPool` classes. The `ThreadSafeQueue` example shows how to push, pop, and manage elements in a thread-safe manner. The `ThreadPool` example demonstrates how to enqueue tasks, retrieve results, and wait for task completion.

Remember to include proper error handling and consider the performance implications of using these classes in your actual implementations.
