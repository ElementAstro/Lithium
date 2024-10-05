# ObjectPool Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructor](#constructor)
4. [Public Methods](#public-methods)
5. [Usage Examples](#usage-examples)

## Introduction

The `ObjectPool` class is a thread-safe implementation of an object pool designed to manage reusable objects. It provides efficient allocation and deallocation of objects, reducing the overhead of object creation and destruction. This class is particularly useful in scenarios where objects are frequently created and destroyed, such as in multi-threaded applications or high-performance systems.

## Class Overview

```cpp
template <Resettable T>
class ObjectPool {
public:
    using CreateFunc = std::function<std::shared_ptr<T>()>;

    explicit ObjectPool(size_t max_size, CreateFunc creator = []() { return std::make_shared<T>(); });

    [[nodiscard]] auto acquire() -> std::shared_ptr<T>;
    template <typename Rep, typename Period>
    [[nodiscard]] auto acquireFor(const std::chrono::duration<Rep, Period>& timeout_duration) -> std::optional<std::shared_ptr<T>>;
    void release(std::shared_ptr<T> obj);
    [[nodiscard]] auto available() const -> size_t;
    [[nodiscard]] auto size() const -> size_t;
    void prefill(size_t count);
    void clear();
    void resize(size_t new_max_size);
    void applyToAll(const std::function<void(T&)>& func);

    // ... (private members)
};
```

The `ObjectPool` class is templated on type `T`, which must satisfy the `Resettable` concept, meaning it must have a `reset()` method.

## Constructor

```cpp
explicit ObjectPool(size_t max_size, CreateFunc creator = []() { return std::make_shared<T>(); })
```

- `max_size`: The maximum number of objects the pool can hold.
- `creator`: An optional function to create new objects. Defaults to using `std::make_shared<T>()`.

## Public Methods

### acquire

```cpp
[[nodiscard]] auto acquire() -> std::shared_ptr<T>
```

Acquires an object from the pool. If no objects are available, it blocks until one becomes available.

**Returns:** A shared pointer to the acquired object.
**Throws:** `std::runtime_error` if the pool is full and no object is available.

### acquireFor

```cpp
template <typename Rep, typename Period>
[[nodiscard]] auto acquireFor(const std::chrono::duration<Rep, Period>& timeout_duration) -> std::optional<std::shared_ptr<T>>
```

Acquires an object from the pool with a timeout.

**Parameters:**

- `timeout_duration`: The maximum duration to wait for an available object.

**Returns:** An optional containing a shared pointer to the acquired object, or `std::nullopt` if the timeout expires.

### release

```cpp
void release(std::shared_ptr<T> obj)
```

Releases an object back to the pool.

**Parameters:**

- `obj`: The shared pointer to the object to release.

### available

```cpp
[[nodiscard]] auto available() const -> size_t
```

Returns the number of available objects in the pool.

### size

```cpp
[[nodiscard]] auto size() const -> size_t
```

Returns the current size of the pool (total number of objects).

### prefill

```cpp
void prefill(size_t count)
```

Prefills the pool with a specified number of objects.

**Parameters:**

- `count`: The number of objects to prefill the pool with.

### clear

```cpp
void clear()
```

Clears all objects from the pool.

### resize

```cpp
void resize(size_t new_max_size)
```

Resizes the pool to a new maximum size.

**Parameters:**

- `new_max_size`: The new maximum size for the pool.

### applyToAll

```cpp
void applyToAll(const std::function<void(T&)>& func)
```

Applies a function to all objects in the pool.

**Parameters:**

- `func`: The function to apply to each object.

## Usage Examples

### Basic Usage

```cpp
#include "object_pool.hpp"
#include <iostream>

class MyObject {
public:
    void reset() { data = 0; }
    int data = 0;
};

int main() {
    ObjectPool<MyObject> pool(5);

    // Acquire and use an object
    auto obj1 = pool.acquire();
    obj1->data = 42;
    std::cout << "Object 1 data: " << obj1->data << std::endl;

    // Release the object back to the pool
    pool.release(obj1);

    // Acquire another object (might be the same one)
    auto obj2 = pool.acquire();
    std::cout << "Object 2 data: " << obj2->data << std::endl;

    return 0;
}
```

### Using with Custom Creator Function

```cpp
#include "object_pool.hpp"
#include <iostream>
#include <random>

class ComplexObject {
public:
    ComplexObject(int id) : id_(id) {}
    void reset() { data_.clear(); }
    void addData(int value) { data_.push_back(value); }
    void printData() const {
        std::cout << "Object " << id_ << " data: ";
        for (int val : data_) {
            std::cout << val << " ";
        }
        std::cout << std::endl;
    }

private:
    int id_;
    std::vector<int> data_;
};

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);

    auto creator = [&gen, &dis]() {
        return std::make_shared<ComplexObject>(dis(gen));
    };

    ObjectPool<ComplexObject> pool(3, creator);

    // Prefill the pool
    pool.prefill(3);

    // Use objects from the pool
    for (int i = 0; i < 5; ++i) {
        auto obj = pool.acquire();
        obj->addData(dis(gen));
        obj->addData(dis(gen));
        obj->printData();
        pool.release(obj);
    }

    return 0;
}
```

### Thread-safe Usage

```cpp
#include "object_pool.hpp"
#include <iostream>
#include <thread>
#include <vector>

class ThreadSafeObject {
public:
    void reset() { counter_ = 0; }
    void increment() { ++counter_; }
    int getCounter() const { return counter_; }

private:
    int counter_ = 0;
};

int main() {
    ObjectPool<ThreadSafeObject> pool(10);
    pool.prefill(5);

    std::vector<std::thread> threads;
    for (int i = 0; i < 20; ++i) {
        threads.emplace_back([&pool]() {
            for (int j = 0; j < 100; ++j) {
                auto obj = pool.acquire();
                obj->increment();
                pool.release(obj);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    pool.applyToAll([](ThreadSafeObject& obj) {
        std::cout << "Object counter: " << obj.getCounter() << std::endl;
    });

    return 0;
}
```
