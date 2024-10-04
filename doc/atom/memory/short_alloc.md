# ShortAlloc Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Arena Class](#arena-class)
3. [ShortAlloc Class](#shortalloc-class)
4. [allocateUnique Function](#allocateunique-function)
5. [Usage Examples](#usage-examples)

## Introduction

The `ShortAlloc` library provides a fixed-size memory allocator designed for scenarios where performance and memory fragmentation are critical concerns. It includes two main components: the `Arena` class, which manages a fixed-size memory buffer, and the `ShortAlloc` class, which serves as an allocator interface compatible with C++ standard library containers.

## Arena Class

The `Arena` class manages a fixed-size memory buffer and provides allocation and deallocation functionality.

### Template Parameters

- `N`: The size of the fixed-size memory arena in bytes.
- `alignment`: The alignment requirement for memory allocations (default: `alignof(std::max_align_t)`).

### Public Methods

```cpp
Arena() ATOM_NOEXCEPT;
~Arena();
void* allocate(std::size_t n);
void deallocate(void* p, std::size_t n) ATOM_NOEXCEPT;
static ATOM_CONSTEXPR std::size_t size() ATOM_NOEXCEPT;
std::size_t used() const ATOM_NOEXCEPT;
void reset() ATOM_NOEXCEPT;
```

## ShortAlloc Class

The `ShortAlloc` class is an allocator that uses the `Arena` for memory management. It's designed to be used with C++ standard library containers.

### Template Parameters

- `T`: The type of objects to allocate.
- `N`: The size of the fixed-size memory arena in bytes.
- `Align`: The alignment requirement for memory allocations (default: `alignof(std::max_align_t)`).

### Public Methods

```cpp
explicit ShortAlloc(arena_type& a) ATOM_NOEXCEPT;
template <class U>
explicit ShortAlloc(const ShortAlloc<U, N, ALIGNMENT>& a) ATOM_NOEXCEPT;
T* allocate(std::size_t n);
void deallocate(T* p, std::size_t n) ATOM_NOEXCEPT;
template <class U, class... Args>
void construct(U* p, Args&&... args);
template <class U>
void destroy(U* p);
```

## allocateUnique Function

The `allocateUnique` function creates a `std::unique_ptr` using a custom allocator.

```cpp
template <typename Alloc, typename T, typename... Args>
std::unique_ptr<T, std::function<void(T*)>> allocateUnique(Alloc& alloc, Args&&... args);
```

## Usage Examples

### Basic Usage with std::vector

```cpp
#include "short_alloc.hpp"
#include <vector>
#include <iostream>

int main() {
    constexpr std::size_t N = 1024; // Size of the arena in bytes
    atom::memory::Arena<N> arena;

    using IntAlloc = atom::memory::ShortAlloc<int, N>;
    std::vector<int, IntAlloc> vec((IntAlloc(arena)));

    for (int i = 0; i < 10; ++i) {
        vec.push_back(i);
    }

    for (int num : vec) {
        std::cout << num << " ";
    }
    std::cout << std::endl;

    return 0;
}
```

### Using ShortAlloc with std::map

```cpp
#include "short_alloc.hpp"
#include <map>
#include <string>
#include <iostream>

int main() {
    constexpr std::size_t N = 4096; // Size of the arena in bytes
    atom::memory::Arena<N> arena;

    using PairType = std::pair<const std::string, int>;
    using MapAlloc = atom::memory::ShortAlloc<PairType, N>;
    std::map<std::string, int, std::less<>, MapAlloc> myMap((MapAlloc(arena)));

    myMap["one"] = 1;
    myMap["two"] = 2;
    myMap["three"] = 3;

    for (const auto& [key, value] : myMap) {
        std::cout << key << ": " << value << std::endl;
    }

    return 0;
}
```

### Using allocateUnique

```cpp
#include "short_alloc.hpp"
#include <iostream>

class MyClass {
public:
    MyClass(int value) : value_(value) {
        std::cout << "MyClass constructed with value: " << value_ << std::endl;
    }
    ~MyClass() {
        std::cout << "MyClass destructed with value: " << value_ << std::endl;
    }
    int getValue() const { return value_; }

private:
    int value_;
};

int main() {
    constexpr std::size_t N = 1024;
    atom::memory::Arena<N> arena;
    atom::memory::ShortAlloc<MyClass, N> alloc(arena);

    auto ptr = atom::memory::allocateUnique<decltype(alloc), MyClass>(alloc, 42);

    std::cout << "Value: " << ptr->getValue() << std::endl;

    return 0;
}
```

### Measuring Arena Usage

```cpp
#include "short_alloc.hpp"
#include <vector>
#include <iostream>

int main() {
    constexpr std::size_t N = 1024;
    atom::memory::Arena<N> arena;

    using IntAlloc = atom::memory::ShortAlloc<int, N>;
    std::vector<int, IntAlloc> vec((IntAlloc(arena)));

    std::cout << "Initial arena usage: " << arena.used() << " bytes" << std::endl;

    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
    }

    std::cout << "Arena usage after adding 100 ints: " << arena.used() << " bytes" << std::endl;

    vec.clear();
    arena.reset();

    std::cout << "Arena usage after reset: " << arena.used() << " bytes" << std::endl;

    return 0;
}
```
