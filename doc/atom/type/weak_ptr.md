# EnhancedWeakPtr Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructor and Destructor](#constructor-and-destructor)
4. [Copy and Move Operations](#copy-and-move-operations)
5. [Basic Operations](#basic-operations)
6. [Advanced Features](#advanced-features)
7. [Static Methods](#static-methods)
8. [Helper Functions](#helper-functions)
9. [Usage Examples](#usage-examples)
10. [Best Practices and Considerations](#best-practices-and-considerations)

## Introduction

The `EnhancedWeakPtr` class template, defined in the `atom::type` namespace, extends the functionality of `std::weak_ptr`. It provides additional features such as periodic locking attempts, asynchronous locking, and batch operations on groups of weak pointers.

## Class Overview

```cpp
namespace atom::type {

template <typename T>
class EnhancedWeakPtr {
    // ... (class implementation)
};

// ... (helper functions)

}  // namespace atom::type
```

## Constructor and Destructor

### Constructors

```cpp
EnhancedWeakPtr();
explicit EnhancedWeakPtr(const std::shared_ptr<T>& shared);
```

### Destructor

```cpp
~EnhancedWeakPtr();
```

Usage:

```cpp
std::shared_ptr<int> sharedPtr = std::make_shared<int>(42);
atom::type::EnhancedWeakPtr<int> weakPtr(sharedPtr);
```

## Copy and Move Operations

```cpp
EnhancedWeakPtr(const EnhancedWeakPtr& other);
EnhancedWeakPtr(EnhancedWeakPtr&& other) noexcept;
auto operator=(const EnhancedWeakPtr& other) -> EnhancedWeakPtr&;
auto operator=(EnhancedWeakPtr&& other) noexcept -> EnhancedWeakPtr&;
```

Usage:

```cpp
atom::type::EnhancedWeakPtr<int> weakPtr1(sharedPtr);
atom::type::EnhancedWeakPtr<int> weakPtr2 = weakPtr1;  // Copy
atom::type::EnhancedWeakPtr<int> weakPtr3 = std::move(weakPtr1);  // Move
```

## Basic Operations

### lock

```cpp
auto lock() const -> std::shared_ptr<T>;
```

Usage:

```cpp
if (auto sharedPtr = weakPtr.lock()) {
    // Use sharedPtr
}
```

### expired

```cpp
auto expired() const -> bool;
```

Usage:

```cpp
if (!weakPtr.expired()) {
    // Weak pointer is still valid
}
```

### reset

```cpp
void reset();
```

Usage:

```cpp
weakPtr.reset();
```

### useCount

```cpp
auto useCount() const -> long;
```

Usage:

```cpp
long count = weakPtr.useCount();
```

## Advanced Features

### withLock

```cpp
template <typename Func, typename R = std::invoke_result_t<Func, T&>>
auto withLock(Func&& func) const
    -> std::conditional_t<std::is_void_v<R>, bool, std::optional<R>>;
```

Usage:

```cpp
weakPtr.withLock([](int& value) {
    std::cout << "Value: " << value << std::endl;
});
```

### waitFor

```cpp
template <typename Rep, typename Period>
auto waitFor(const std::chrono::duration<Rep, Period>& timeout) const -> bool;
```

Usage:

```cpp
if (weakPtr.waitFor(std::chrono::seconds(5))) {
    // Object became available within 5 seconds
}
```

### tryLockOrElse

```cpp
template <typename SuccessFunc, typename FailureFunc>
auto tryLockOrElse(SuccessFunc&& success, FailureFunc&& failure) const;
```

Usage:

```cpp
int result = weakPtr.tryLockOrElse(
    [](int& value) { return value * 2; },
    [] { return -1; }
);
```

### tryLockPeriodic

```cpp
template <typename Rep, typename Period>
auto tryLockPeriodic(
    const std::chrono::duration<Rep, Period>& interval,
    size_t maxAttempts = std::numeric_limits<size_t>::max()) -> std::shared_ptr<T>;
```

Usage:

```cpp
auto sharedPtr = weakPtr.tryLockPeriodic(std::chrono::milliseconds(100), 10);
```

### asyncLock

```cpp
auto asyncLock() const -> std::future<std::shared_ptr<T>>;
```

Usage:

```cpp
auto future = weakPtr.asyncLock();
// ... do other work ...
auto sharedPtr = future.get();
```

### waitUntil

```cpp
template <typename Predicate>
auto waitUntil(Predicate pred) const -> bool;
```

Usage:

```cpp
bool result = weakPtr.waitUntil([] { return someCondition; });
```

### cast

```cpp
template <typename U>
auto cast() const -> EnhancedWeakPtr<U>;
```

Usage:

```cpp
auto derivedWeakPtr = baseWeakPtr.cast<Derived>();
```

## Static Methods

### getTotalInstances

```cpp
static auto getTotalInstances() -> size_t;
```

Usage:

```cpp
size_t totalInstances = atom::type::EnhancedWeakPtr<int>::getTotalInstances();
```

## Helper Functions

### createWeakPtrGroup

```cpp
template <typename T>
auto createWeakPtrGroup(const std::vector<std::shared_ptr<T>>& sharedPtrs)
    -> std::vector<EnhancedWeakPtr<T>>;
```

Usage:

```cpp
std::vector<std::shared_ptr<int>> sharedPtrs = { /* ... */ };
auto weakPtrGroup = atom::type::createWeakPtrGroup(sharedPtrs);
```

### batchOperation

```cpp
template <typename T, typename Func>
void batchOperation(const std::vector<EnhancedWeakPtr<T>>& weakPtrs, Func&& func);
```

Usage:

```cpp
atom::type::batchOperation(weakPtrGroup, [](int& value) {
    value *= 2;
});
```

## Usage Examples

### Example 1: Basic Usage

```cpp
#include "weak_ptr.hpp"
#include <iostream>

int main() {
    auto sharedPtr = std::make_shared<int>(42);
    atom::type::EnhancedWeakPtr<int> weakPtr(sharedPtr);

    weakPtr.withLock([](int& value) {
        std::cout << "Value: " << value << std::endl;
    });

    sharedPtr.reset();  // Release the shared pointer

    if (weakPtr.expired()) {
        std::cout << "Weak pointer has expired." << std::endl;
    }

    return 0;
}
```

### Example 2: Asynchronous Operations

```cpp
#include "weak_ptr.hpp"
#include <iostream>
#include <thread>

int main() {
    auto sharedPtr = std::make_shared<int>(0);
    atom::type::EnhancedWeakPtr<int> weakPtr(sharedPtr);

    std::thread t([&weakPtr]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        weakPtr.withLock([](int& value) {
            value = 42;
        });
        weakPtr.notifyAll();
    });

    bool result = weakPtr.waitUntil([]() {
        return true;  // Wait until notified
    });

    if (result) {
        weakPtr.withLock([](int& value) {
            std::cout << "Value updated to: " << value << std::endl;
        });
    }

    t.join();
    return 0;
}
```

This example demonstrates the use of `waitUntil` and `notifyAll` for synchronization between threads.

### Example 3: Periodic Locking and Casting

```cpp
#include "weak_ptr.hpp"
#include <iostream>
#include <chrono>

class Base {
public:
    virtual ~Base() = default;
    virtual void print() const { std::cout << "Base" << std::endl; }
};

class Derived : public Base {
public:
    void print() const override { std::cout << "Derived" << std::endl; }
};

int main() {
    auto sharedPtr = std::make_shared<Derived>();
    atom::type::EnhancedWeakPtr<Base> baseWeakPtr(sharedPtr);

    // Simulate the shared pointer being temporarily unavailable
    std::thread([&sharedPtr]() {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        sharedPtr.reset();
    }).detach();

    // Try to lock periodically
    auto lockedPtr = baseWeakPtr.tryLockPeriodic(std::chrono::milliseconds(500), 10);
    if (lockedPtr) {
        lockedPtr->print();  // Outputs: Derived

        // Cast to Derived
        auto derivedWeakPtr = baseWeakPtr.cast<Derived>();
        derivedWeakPtr.withLock([](Derived& obj) {
            obj.print();  // Outputs: Derived
        });
    } else {
        std::cout << "Failed to lock after multiple attempts" << std::endl;
    }

    return 0;
}
```

This example shows how to use `tryLockPeriodic` for periodic locking attempts and `cast` for downcasting.

### Example 4: Batch Operations

```cpp
#include "weak_ptr.hpp"
#include <iostream>
#include <vector>

class Resource {
public:
    Resource(int id) : id_(id) {}
    void update() { std::cout << "Updating resource " << id_ << std::endl; }
private:
    int id_;
};

int main() {
    std::vector<std::shared_ptr<Resource>> resources;
    for (int i = 0; i < 5; ++i) {
        resources.push_back(std::make_shared<Resource>(i));
    }

    auto weakPtrGroup = atom::type::createWeakPtrGroup(resources);

    // Perform batch operation
    atom::type::batchOperation(weakPtrGroup, [](Resource& res) {
        res.update();
    });

    // Simulate some resources being released
    resources[1].reset();
    resources[3].reset();

    // Perform another batch operation
    atom::type::batchOperation(weakPtrGroup, [](Resource& res) {
        res.update();
    });

    return 0;
}
```

This example demonstrates how to use `createWeakPtrGroup` and `batchOperation` to perform operations on a group of objects.

## Best Practices and Considerations

1. **Use `EnhancedWeakPtr` when appropriate**: Use `EnhancedWeakPtr` when you need the additional functionality it provides over `std::weak_ptr`. For simple cases, `std::weak_ptr` may be sufficient.

2. **Avoid circular references**: Even with `EnhancedWeakPtr`, be careful to avoid circular references that could lead to memory leaks.

3. **Handle expiration gracefully**: Always check if the weak pointer has expired before using it. Use the `withLock` method to safely access the object and handle the case when it's no longer available.

4. **Be mindful of performance**: Some operations, like `tryLockPeriodic`, can be potentially expensive if not used carefully. Consider the performance implications in your specific use case.

5. **Use `asyncLock` for non-blocking operations**: When you don't want to block the current thread waiting for a lock, use `asyncLock` to perform the locking operation asynchronously.

6. **Leverage batch operations**: When dealing with multiple objects, use `createWeakPtrGroup` and `batchOperation` to perform operations efficiently.

7. **Be cautious with `cast`**: When using `cast`, ensure that the cast is valid. Incorrect casts can lead to undefined behavior.

8. **Use `waitFor` and `waitUntil` judiciously**: These methods can be useful for synchronization, but be careful not to create deadlocks or long waits that could impact performance.

9. **Monitor lock attempts**: Use `getLockAttempts` to track how often locking is attempted, which can be useful for debugging or optimizing your code.

10. **Consider thread safety**: While `EnhancedWeakPtr` provides some thread-safe operations, be aware of potential race conditions when using shared resources across multiple threads.

11. **Use RAII**: When possible, use RAII (Resource Acquisition Is Initialization) principles to manage the lifetime of your objects, which can help prevent issues with dangling pointers.

12. **Understand the memory model**: Be aware of the C++ memory model and how it affects the behavior of weak pointers, especially in multi-threaded environments.

## Conclusion

The `EnhancedWeakPtr` class provides a powerful extension to the standard `std::weak_ptr`, offering additional functionality for complex scenarios involving shared ownership and asynchronous operations. By understanding its features and following best practices, you can leverage `EnhancedWeakPtr` to write more robust and flexible C++ code.

Remember to always consider the specific requirements of your project and the potential performance implications when choosing between `std::weak_ptr` and `EnhancedWeakPtr`. In many cases, the standard `std::weak_ptr` may be sufficient, but `EnhancedWeakPtr` can be a valuable tool when you need its advanced features.
