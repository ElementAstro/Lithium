# Extra Locks

## Spinlock

A simple spinlock implementation using atomic_flag.

### Class Definition

```cpp
class Spinlock {
    // ...
public:
    Spinlock() = default;
    Spinlock(const Spinlock &) = delete;
    Spinlock &operator=(const Spinlock &) = delete;

    void lock();
    void unlock();
};
```

### Usage Example

```cpp
Spinlock spinlock;

// Lock the spinlock
spinlock.lock();

// Critical section

// Unlock the spinlock
spinlock.unlock();
```

## TicketSpinlock

A ticket spinlock implementation using atomic operations.

### Class Definition

```cpp
class TicketSpinlock {
    // ...
public:
    TicketSpinlock() = default;
    TicketSpinlock(const TicketSpinlock &) = delete;
    TicketSpinlock &operator=(const TicketSpinlock &) = delete;

    class LockGuard {
        // ...
    };

    using scoped_lock = LockGuard;

    uint64_t lock();
    void unlock(const uint64_t ticket);
};
```

### Usage Example

```cpp
TicketSpinlock ticketSpinlock;

// Create a lock guard and acquire the lock
{
    TicketSpinlock::LockGuard lockGuard(ticketSpinlock);

    // Critical section
}

// Lock and unlock using ticket numbers
uint64_t ticket = ticketSpinlock.lock();

// Critical section

ticketSpinlock.unlock(ticket);
```

## UnfairSpinlock

An unfair spinlock implementation using atomic_flag.

### Class Definition

```cpp
class UnfairSpinlock {
    // ...
public:
    UnfairSpinlock() = default;
    UnfairSpinlock(const UnfairSpinlock &) = delete;
    UnfairSpinlock &operator=(const UnfairSpinlock &) = delete;

    void lock();
    void unlock();
};
```

### Usage Example

```cpp
UnfairSpinlock unfairSpinlock;

// Lock the spinlock
unfairSpinlock.lock();

// Critical section

// Unlock the spinlock
unfairSpinlock.unlock();
```

## ScopedLock

Scoped lock for any type of spinlock.

### Class Definition

```cpp
template <typename Mutex>
class ScopedLock {
    // ...
public:
    explicit ScopedLock(Mutex &mutex);
    ~ScopedLock();
};
```

### Usage Example

```cpp
Spinlock spinlock;

// Acquire the lock using scoped lock
{
    ScopedLock<Spinlock> scopedLock(spinlock);

    // Critical section
}
// Lock will be automatically released when 'scopedLock' goes out of scope
```
