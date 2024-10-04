# Documentation for lock.hpp

This document provides a detailed overview of the `lock.hpp` header file, which contains various spinlock implementations and associated scoped lock classes for use in multi-threaded C++ applications.

## Table of Contents

1. [CPU Relax Macro](#cpu-relax-macro)
2. [Spinlock Class](#spinlock-class)
3. [TicketSpinlock Class](#ticketspinlock-class)
4. [UnfairSpinlock Class](#unfairspinlock-class)
5. [ScopedLock Class Template](#scopedlock-class-template)
6. [ScopedTicketLock Class Template](#scopedticketlock-class-template)
7. [ScopedUnfairLock Class Template](#scopedunfairlock-class-template)
8. [Usage Examples](#usage-examples)

## CPU Relax Macro

The header defines a `cpu_relax()` macro for different architectures to prevent excess processor bus usage:

```cpp
#if defined(_MSC_VER)
#define cpu_relax() std::this_thread::yield()
#elif defined(__i386__) || defined(__x86_64__)
#define cpu_relax() asm volatile("pause\n" : : : "memory")
#elif defined(__aarch64__)
#define cpu_relax() asm volatile("yield\n" : : : "memory")
#elif defined(__arm__)
#define cpu_relax() asm volatile("nop\n" : : : "memory")
#else
#error "Unknown architecture, CPU relax code required"
#endif
```

## Spinlock Class

A simple spinlock implementation using `std::atomic_flag`.

### Methods

```cpp
void lock();
void unlock();
bool tryLock();
```

- `lock()`: Acquires the lock.
- `unlock()`: Releases the lock.
- `tryLock()`: Tries to acquire the lock, returns true if successful.

## TicketSpinlock Class

A ticket-based spinlock implementation using atomic operations.

### Methods

```cpp
uint64_t lock();
void unlock(uint64_t ticket);
```

- `lock()`: Acquires the lock and returns the ticket number.
- `unlock(uint64_t ticket)`: Releases the lock for the given ticket number.

### Nested LockGuard Class

```cpp
class LockGuard {
public:
    explicit LockGuard(TicketSpinlock& spinlock);
    ~LockGuard();
};
```

A RAII-style lock guard for `TicketSpinlock`.

## UnfairSpinlock Class

An unfair spinlock implementation using `std::atomic_flag`.

### Methods

```cpp
void lock();
void unlock();
```

- `lock()`: Acquires the lock.
- `unlock()`: Releases the lock.

## ScopedLock Class Template

A generic scoped lock for any type of spinlock.

### Methods

```cpp
template <typename Mutex>
class ScopedLock {
public:
    explicit ScopedLock(Mutex& mutex);
    ~ScopedLock();
};
```

- Constructor: Acquires the lock on the provided mutex.
- Destructor: Releases the lock.

## ScopedTicketLock Class Template

A scoped lock specifically for `TicketSpinlock`.

### Methods

```cpp
template <typename Mutex>
class ScopedTicketLock : public NonCopyable {
public:
    explicit ScopedTicketLock(Mutex& mutex);
    ~ScopedTicketLock();
};
```

- Constructor: Acquires the lock on the provided `TicketSpinlock`.
- Destructor: Releases the lock using the stored ticket.

## ScopedUnfairLock Class Template

A scoped lock specifically for `UnfairSpinlock`.

### Methods

```cpp
template <typename Mutex>
class ScopedUnfairLock : public NonCopyable {
public:
    explicit ScopedUnfairLock(Mutex& mutex);
    ~ScopedUnfairLock();
};
```

- Constructor: Acquires the lock on the provided `UnfairSpinlock`.
- Destructor: Releases the lock.

## Usage Examples

Here are some examples demonstrating how to use the various lock types:

### Using Spinlock

```cpp
#include "lock.hpp"
#include <iostream>
#include <thread>

atom::async::Spinlock spinlock;
int shared_resource = 0;

void increment() {
    for (int i = 0; i < 1000000; ++i) {
        spinlock.lock();
        ++shared_resource;
        spinlock.unlock();
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);

    t1.join();
    t2.join();

    std::cout << "Final value: " << shared_resource << std::endl;
    return 0;
}
```

### Using TicketSpinlock

```cpp
#include "lock.hpp"
#include <iostream>
#include <thread>

atom::async::TicketSpinlock ticketlock;
int shared_resource = 0;

void increment() {
    for (int i = 0; i < 1000000; ++i) {
        atom::async::TicketSpinlock::LockGuard guard(ticketlock);
        ++shared_resource;
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);

    t1.join();
    t2.join();

    std::cout << "Final value: " << shared_resource << std::endl;
    return 0;
}
```

### Using UnfairSpinlock with ScopedUnfairLock

```cpp
#include "lock.hpp"
#include <iostream>
#include <thread>

atom::async::UnfairSpinlock unfairlock;
int shared_resource = 0;

void increment() {
    for (int i = 0; i < 1000000; ++i) {
        atom::async::ScopedUnfairLock<atom::async::UnfairSpinlock> guard(unfairlock);
        ++shared_resource;
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);

    t1.join();
    t2.join();

    std::cout << "Final value: " << shared_resource << std::endl;
    return 0;
}
```

These examples demonstrate the basic usage of the different spinlock types and their associated scoped locks. Remember to include proper error handling and consider the performance implications of spinlocks in your actual implementations.
