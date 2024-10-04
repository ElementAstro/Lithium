# RingBuffer Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructor](#constructor)
4. [Public Methods](#public-methods)
5. [Iterator](#iterator)
6. [Usage Examples](#usage-examples)

## Introduction

The `RingBuffer` class is a circular buffer implementation in C++. It provides an efficient way to store and manage a fixed-size sequence of elements with automatic overwriting of old data when the buffer is full. This data structure is particularly useful in scenarios where you need to maintain a rolling window of the most recent data, such as in signal processing, data streaming, or implementing caches.

## Class Overview

```cpp
template <typename T>
class RingBuffer {
public:
    explicit RingBuffer(size_t size);
    // ... (public methods)
private:
    std::vector<T> buffer_;
    size_t max_size_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t count_ = 0;
};
```

The `RingBuffer` class is templated on the type `T` of elements it will store.

## Constructor

```cpp
explicit RingBuffer(size_t size)
```

Creates a new `RingBuffer` with the specified maximum size.

- `size`: The maximum number of elements the buffer can hold.

## Public Methods

### push

```cpp
auto push(const T& item) -> bool
```

Pushes an item to the buffer. Returns `true` if successful, `false` if the buffer is full.

### pushOverwrite

```cpp
void pushOverwrite(const T& item)
```

Pushes an item to the buffer, overwriting the oldest item if the buffer is full.

### pop

```cpp
[[nodiscard]] auto pop() -> std::optional<T>
```

Removes and returns the oldest item from the buffer. Returns `std::nullopt` if the buffer is empty.

### full

```cpp
[[nodiscard]] auto full() const -> bool
```

Returns `true` if the buffer is full, `false` otherwise.

### empty

```cpp
[[nodiscard]] auto empty() const -> bool
```

Returns `true` if the buffer is empty, `false` otherwise.

### size

```cpp
[[nodiscard]] auto size() const -> size_t
```

Returns the current number of items in the buffer.

### capacity

```cpp
[[nodiscard]] auto capacity() const -> size_t
```

Returns the maximum size of the buffer.

### clear

```cpp
void clear()
```

Removes all items from the buffer.

### front

```cpp
[[nodiscard]] auto front() const -> std::optional<T>
```

Returns the oldest item in the buffer without removing it. Returns `std::nullopt` if the buffer is empty.

### back

```cpp
[[nodiscard]] auto back() const -> std::optional<T>
```

Returns the newest item in the buffer without removing it. Returns `std::nullopt` if the buffer is empty.

### contains

```cpp
[[nodiscard]] auto contains(const T& item) const -> bool
```

Returns `true` if the item is in the buffer, `false` otherwise.

### view

```cpp
[[nodiscard]] auto view() const -> std::vector<T>
```

Returns a vector containing all items in the buffer in order.

### resize

```cpp
void resize(size_t new_size)
```

Resizes the buffer to the new size, preserving as many elements as possible.

### at

```cpp
[[nodiscard]] auto at(size_t index) const -> std::optional<T>
```

Returns the element at the specified index. Returns `std::nullopt` if the index is out of bounds.

### forEach

```cpp
template <std::invocable<T&> F>
void forEach(F&& func)
```

Applies the given function to each element in the buffer.

### removeIf

```cpp
template <std::predicate<T> P>
void removeIf(P&& pred)
```

Removes all elements from the buffer that satisfy the given predicate.

### rotate

```cpp
void rotate(int n)
```

Rotates the buffer by `n` positions. Positive values rotate left, negative values rotate right.

## Iterator

The `RingBuffer` class provides a forward iterator for traversing the elements in the buffer.

```cpp
auto begin() const -> Iterator
auto end() const -> Iterator
```

These methods return iterators to the beginning and end of the buffer, respectively.

## Usage Examples

### Basic Usage

```cpp
#include "ring_buffer.hpp"
#include <iostream>

int main() {
    RingBuffer<int> buffer(5);

    // Push elements
    for (int i = 1; i <= 7; ++i) {
        buffer.pushOverwrite(i);
    }

    // Print buffer contents
    std::cout << "Buffer contents: ";
    for (const auto& item : buffer) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    // Pop elements
    while (!buffer.empty()) {
        std::cout << "Popped: " << buffer.pop().value() << std::endl;
    }

    return 0;
}
```

Output:

```
Buffer contents: 3 4 5 6 7
Popped: 3
Popped: 4
Popped: 5
Popped: 6
Popped: 7
```

### Using forEach and removeIf

```cpp
#include "ring_buffer.hpp"
#include <iostream>

int main() {
    RingBuffer<int> buffer(10);

    // Fill the buffer
    for (int i = 1; i <= 10; ++i) {
        buffer.push(i);
    }

    // Double all even numbers
    buffer.forEach([](int& x) {
        if (x % 2 == 0) x *= 2;
    });

    // Remove odd numbers
    buffer.removeIf([](int x) { return x % 2 != 0; });

    // Print remaining elements
    std::cout << "Even numbers doubled: ";
    for (const auto& item : buffer) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

Output:

```
Even numbers doubled: 4 8 12 16 20
```

### Using rotate

```cpp
#include "ring_buffer.hpp"
#include <iostream>

int main() {
    RingBuffer<char> buffer(5);

    // Fill the buffer
    for (char c = 'A'; c <= 'E'; ++c) {
        buffer.push(c);
    }

    std::cout << "Original: ";
    for (const auto& item : buffer) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    // Rotate left by 2
    buffer.rotate(2);

    std::cout << "Rotated left by 2: ";
    for (const auto& item : buffer) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    // Rotate right by 3
    buffer.rotate(-3);

    std::cout << "Rotated right by 3: ";
    for (const auto& item : buffer) {
        std::cout << item << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

Output:

```
Original: A B C D E
Rotated left by 2: C D E A B
Rotated right by 3: E A B C D
```
