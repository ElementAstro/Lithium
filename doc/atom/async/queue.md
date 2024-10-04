# Documentation for queue.hpp

This document provides a detailed overview of the `queue.hpp` header file, which contains the `ThreadSafeQueue` class template for implementing a thread-safe queue in C++.

## Table of Contents

1. [ThreadSafeQueue Class Template](#threadsafequeue-class-template)
2. [Public Methods](#public-methods)
3. [Usage Examples](#usage-examples)

## ThreadSafeQueue Class Template

The `ThreadSafeQueue` class template provides a thread-safe implementation of a queue data structure.

### Template Parameter

- `T`: The type of elements stored in the queue.

### Key Features

- Thread-safe operations
- Blocking and non-blocking element retrieval
- Support for predicates and transformations
- Parallel processing capabilities

## Public Methods

### Basic Queue Operations

#### put

```cpp
void put(T element);
```

Adds an element to the back of the queue.

#### take

```cpp
auto take() -> std::optional<T>;
```

Removes and returns the front element of the queue. Blocks if the queue is empty.

#### tryTake

```cpp
auto tryTake() -> std::optional<T>;
```

Tries to remove and return the front element of the queue without blocking.

#### destroy

```cpp
auto destroy() -> std::queue<T>;
```

Destroys the queue and returns its contents.

#### size

```cpp
[[nodiscard]] auto size() const -> size_t;
```

Returns the number of elements in the queue.

#### empty

```cpp
[[nodiscard]] auto empty() const -> bool;
```

Checks if the queue is empty.

#### clear

```cpp
void clear();
```

Removes all elements from the queue.

#### front

```cpp
auto front() -> std::optional<T>;
```

Returns the front element without removing it.

#### back

```cpp
auto back() -> std::optional<T>;
```

Returns the back element without removing it.

#### emplace

```cpp
template <typename... Args>
void emplace(Args&&... args);
```

Constructs an element in-place at the back of the queue.

### Advanced Operations

#### waitFor

```cpp
template <std::predicate<const T&> Predicate>
auto waitFor(Predicate predicate) -> std::optional<T>;
```

Waits for an element satisfying the given predicate and returns it.

#### waitUntilEmpty

```cpp
void waitUntilEmpty();
```

Waits until the queue becomes empty.

#### extractIf

```cpp
template <std::predicate<const T&> UnaryPredicate>
auto extractIf(UnaryPredicate pred) -> std::vector<T>;
```

Extracts all elements satisfying the given predicate.

#### sort

```cpp
template <typename Compare>
void sort(Compare comp);
```

Sorts the elements in the queue using the provided comparison function.

#### transform

```cpp
template <typename ResultType>
auto transform(std::function<ResultType(T)> func)
    -> std::shared_ptr<ThreadSafeQueue<ResultType>>;
```

Applies a transformation function to all elements and returns a new queue.

#### groupBy

```cpp
template <typename GroupKey>
auto groupBy(std::function<GroupKey(const T&)> func)
    -> std::vector<std::shared_ptr<ThreadSafeQueue<T>>>;
```

Groups elements by a key function and returns a vector of queues.

#### toVector

```cpp
auto toVector() const -> std::vector<T>;
```

Returns a vector containing all elements in the queue.

#### forEach

```cpp
template <typename Func>
void forEach(Func func, bool parallel = false);
```

Applies a function to each element in the queue, optionally in parallel.

#### takeFor

```cpp
template <typename Rep, typename Period>
auto takeFor(const std::chrono::duration<Rep, Period>& timeout)
    -> std::optional<T>;
```

Tries to remove and return the front element, waiting for a specified duration.

#### takeUntil

```cpp
template <typename Clock, typename Duration>
auto takeUntil(const std::chrono::time_point<Clock, Duration>& timeout_time)
    -> std::optional<T>;
```

Tries to remove and return the front element, waiting until a specified time point.

## Usage Examples

Here are some examples demonstrating how to use the `ThreadSafeQueue` class:

### Basic Usage

```cpp
#include "queue.hpp"
#include <iostream>

int main() {
    atom::async::ThreadSafeQueue<int> queue;

    // Add elements
    queue.put(1);
    queue.put(2);
    queue.put(3);

    // Remove and print elements
    while (auto item = queue.take()) {
        std::cout << *item << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

### Using Predicates and Transformations

```cpp
atom::async::ThreadSafeQueue<int> queue;
for (int i = 1; i <= 10; ++i) {
    queue.put(i);
}

// Wait for and extract even numbers
auto even = queue.waitFor([](const int& x) { return x % 2 == 0; });
if (even) {
    std::cout << "First even number: " << *even << std::endl;
}

// Extract all odd numbers
auto odds = queue.extractIf([](const int& x) { return x % 2 != 0; });
std::cout << "Odd numbers: ";
for (const auto& odd : odds) {
    std::cout << odd << " ";
}
std::cout << std::endl;

// Transform remaining numbers (squaring them)
auto squared = queue.transform<int>([](int x) { return x * x; });
```

### Parallel Processing

```cpp
atom::async::ThreadSafeQueue<int> queue;
for (int i = 1; i <= 1000; ++i) {
    queue.put(i);
}

// Process elements in parallel
queue.forEach([](int& x) {
    x = x * x;  // Square each number
}, true);  // Set parallel to true

// Print results
queue.forEach([](const int& x) {
    std::cout << x << " ";
});
std::cout << std::endl;
```

### Timed Operations

```cpp
atom::async::ThreadSafeQueue<std::string> queue;

// Try to take an element, waiting for at most 2 seconds
auto item = queue.takeFor(std::chrono::seconds(2));
if (item) {
    std::cout << "Got item: " << *item << std::endl;
} else {
    std::cout << "Timed out waiting for item" << std::endl;
}

// Try to take an element, waiting until a specific time point
auto deadline = std::chrono::steady_clock::now() + std::chrono::minutes(1);
auto future_item = queue.takeUntil(deadline);
if (future_item) {
    std::cout << "Got item before deadline: " << *future_item << std::endl;
} else {
    std::cout << "Deadline passed without getting an item" << std::endl;
}
```
