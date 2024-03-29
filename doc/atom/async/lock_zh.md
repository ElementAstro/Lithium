# 额外的锁

## 自旋锁（Spinlock）

使用原子标志（atomic_flag）实现的简单自旋锁。

### 类定义

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

### 用法示例

```cpp
Spinlock spinlock;

// 锁定自旋锁
spinlock.lock();

// 临界区

// 解锁自旋锁
spinlock.unlock();
```

## 票据自旋锁（TicketSpinlock）

使用原子操作实现的票据自旋锁。

### 类定义

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

### 用法示例

```cpp
TicketSpinlock ticketSpinlock;

// 创建一个锁保护对象并获取锁
{
    TicketSpinlock::LockGuard lockGuard(ticketSpinlock);

    // 临界区
}

// 使用票据号码进行锁定和解锁
uint64_t ticket = ticketSpinlock.lock();

// 临界区

ticketSpinlock.unlock(ticket);
```

## 不公平自旋锁（UnfairSpinlock）

使用原子标志（atomic_flag）实现的不公平自旋锁。

### 类定义

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

### 用法示例

```cpp
UnfairSpinlock unfairSpinlock;

// 锁定不公平自旋锁
unfairSpinlock.lock();

// 临界区

// 解锁不公平自旋锁
unfairSpinlock.unlock();
```

## 作用域锁（ScopedLock）

适用于任何类型自旋锁的作用域锁。

### 类定义

```cpp
template <typename Mutex>
class ScopedLock {
    // ...
public:
    explicit ScopedLock(Mutex &mutex);
    ~ScopedLock();
};
```

### 用法示例

```cpp
Spinlock spinlock;

// 使用作用域锁获取锁
{
    ScopedLock<Spinlock> scopedLock(spinlock);

    // 临界区
}
// 当 'scopedLock' 超出作用域时，锁会自动释放
```
