# MemoryPool Documentation

This document provides a detailed explanation of the `MemoryPool` class template, its methods, and usage examples.

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructor and Destructor](#constructor-and-destructor)
4. [Public Methods](#public-methods)
5. [Protected Methods](#protected-methods)
6. [Private Methods](#private-methods)
7. [Usage Examples](#usage-examples)

## Introduction

The `MemoryPool` class template is a custom memory allocator that implements the `std::pmr::memory_resource` interface. It's designed to efficiently manage memory allocation and deallocation for objects of type `T`, using a block-based allocation strategy.

## Class Overview

```cpp
template <typename T, size_t BlockSize = 4096>
class MemoryPool : public std::pmr::memory_resource, NonCopyable {
    // ... (class contents)
};
```

- `T`: The type of objects to be allocated.
- `BlockSize`: The size of each memory block (default is 4096 bytes).

The class inherits from `std::pmr::memory_resource` and `NonCopyable`, making it a non-copyable polymorphic memory resource.

## Constructor and Destructor

```cpp
MemoryPool() = default;
~MemoryPool() override = default;
```

The constructor and destructor are defaulted, meaning they use the compiler-generated implementations.

## Public Methods

### allocate

```cpp
auto allocate(size_t n) -> T*;
```

Allocates memory for `n` objects of type `T`.

- Parameters:
  - `n`: The number of objects to allocate space for.
- Returns: A pointer to the allocated memory.
- Throws: `std::bad_alloc` if the allocation fails.

### deallocate

```cpp
void deallocate(T* p, size_t n);
```

Deallocates memory previously allocated by `allocate`.

- Parameters:
  - `p`: Pointer to the memory to deallocate.
  - `n`: The number of objects the memory was allocated for.

### do_is_equal

```cpp
[[nodiscard]] auto do_is_equal(const std::pmr::memory_resource& other) const noexcept -> bool override;
```

Checks if this memory resource is equal to another.

- Parameters:
  - `other`: The other memory resource to compare with.
- Returns: `true` if the memory resources are the same object, `false` otherwise.

## Protected Methods

### do_allocate

```cpp
auto do_allocate(size_t bytes, size_t alignment) -> void* override;
```

Allocates memory with the specified size and alignment.

- Parameters:
  - `bytes`: The number of bytes to allocate.
  - `alignment`: The alignment requirement.
- Returns: A pointer to the allocated memory.
- Throws: `std::bad_alloc` if the allocation fails.

### do_deallocate

```cpp
void do_deallocate(void* p, size_t bytes, size_t alignment) override;
```

Deallocates memory previously allocated by `do_allocate`.

- Parameters:
  - `p`: Pointer to the memory to deallocate.
  - `bytes`: The size of the allocation.
  - `alignment`: The alignment of the allocation.

## Private Methods

The class includes several private methods for internal memory management:

- `maxSize()`: Returns the maximum size of a single allocation.
- `chunkSpace()`: Returns the size of a memory chunk.
- `allocateFromPool()`: Attempts to allocate from the existing memory pool.
- `deallocateToPool()`: Deallocates memory back to the pool.
- `allocateFromChunk()`: Allocates a new chunk of memory.
- `deallocateToChunk()`: Deallocates a chunk of memory.
- `isFromPool()`: Checks if a pointer is from the memory pool.

## Usage Examples

### Basic Usage

```cpp
#include "memory_pool.hpp"
#include <iostream>

int main() {
    MemoryPool<int> pool;

    // Allocate memory for 10 integers
    int* numbers = pool.allocate(10);

    // Use the allocated memory
    for (int i = 0; i < 10; ++i) {
        numbers[i] = i * 10;
    }

    // Print the numbers
    for (int i = 0; i < 10; ++i) {
        std::cout << numbers[i] << " ";
    }
    std::cout << std::endl;

    // Deallocate the memory
    pool.deallocate(numbers, 10);

    return 0;
}
```

### Using with Standard Containers

```cpp
#include "memory_pool.hpp"
#include <vector>
#include <memory_resource>

int main() {
    MemoryPool<int> pool;
    std::pmr::polymorphic_allocator<int> alloc(&pool);

    // Create a vector using the custom allocator
    std::vector<int, std::pmr::polymorphic_allocator<int>> vec(alloc);

    // Add elements to the vector
    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
    }

    // Use the vector as normal
    for (int num : vec) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

### Custom Object Allocation

```cpp
#include "memory_pool.hpp"
#include <iostream>

class MyClass {
public:
    MyClass(int val) : value(val) {}
    int getValue() const { return value; }
private:
    int value;
};

int main() {
    MemoryPool<MyClass> pool;

    // Allocate memory for 5 MyClass objects
    MyClass* objects = pool.allocate(5);

    // Construct objects in the allocated memory
    for (int i = 0; i < 5; ++i) {
        new (&objects[i]) MyClass(i * 100);
    }

    // Use the objects
    for (int i = 0; i < 5; ++i) {
        std::cout << "Object " << i << " value: " << objects[i].getValue() << std::endl;
    }

    // Destroy the objects and deallocate memory
    for (int i = 0; i < 5; ++i) {
        objects[i].~MyClass();
    }
    pool.deallocate(objects, 5);

    return 0;
}
```
