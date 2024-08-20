# 临时释放锁：Antilock 模式 (RRII)

## 前言

在多线程编程中，正确管理线程同步是确保程序稳定性和性能的关键。C++ 提供了多种工具来帮助开发者实现线程同步，例如 `std::mutex`、`std::lock_guard` 和 `std::unique_lock` 等。这些工具虽然强大，但在某些复杂场景下，可能需要更灵活的锁管理方式。比如说有的时候我在加锁后，在局部代码中需要释放锁，然后后续运行又需要加锁，这个时候我们虽然可以通过`unlock`和`lock`组合完成，但是代码变长后难免会出现遗忘的情况，从而产生错误。那么，本文将介绍一种名为“Antilock”的模式，它能够在需要时暂时释放锁，并在操作完成后自动重新获取锁，从而避免潜在的死锁和遗忘问题。

本文的理念来自 [Antilock 模式](https://devblogs.microsoft.com/oldnewthing/20240814-00/?p=110129)

## 概念

### 互斥锁与 RAII 模式

在 C++ 中，互斥锁（mutex）用于保护共享资源，防止多个线程同时访问而导致数据不一致。为了简化锁的管理，C++ 标准库引入了 RAII（Resource Acquisition Is Initialization）模式。RAII 模式通过在对象的构造函数中获取资源，在析构函数中释放资源，确保资源管理的安全性。
例如，`std::lock_guard` 就是一个常用的 RAII 类型，它会在构造时自动锁定互斥锁，并在析构时自动解锁：

```cpp
std::mutex mtx;
{
    std::lock_guard<std::mutex> guard(mtx);
    // 这里的代码块在锁的保护下执行
} // 离开作用域时，锁自动释放
```

### Antilock 模式

Antilock 模式是一种反作用的锁管理策略，它的作用是暂时释放一个已经锁定的互斥锁，并在特定操作完成后重新获取该锁。这种模式特别适合需要在锁的保护下执行部分操作，但又需要在某些时候释放锁以避免死锁的场景。

那么这种机制是否可以称为`RRII`呢？即`Resource Release Is Initialization`！

## 实现

### 基本实现

我们首先定义一个简单的 Antilock 模板类，它可以接受一个互斥锁对象，在构造时解锁该互斥锁，并在析构时重新锁定它：

```cpp
template<typename Mutex>
class Antilock {
public:
    Antilock() = default;

    explicit Antilock(Mutex& mutex)
        : m_mutex(std::addressof(mutex)) {
        if (m_mutex) { // 这里做一个检查如果互斥锁有效
            m_mutex->unlock();
            std::cout << "[Antilock] Lock released.\n";
        }
    }

    ~Antilock() {
        if (m_mutex) {
            m_mutex->lock();
            std::cout << "[Antilock] Lock reacquired.\n";
        }
    }

private:
    Mutex* m_mutex = nullptr;  // 指向互斥锁的指针
};
```

这个简单的 `Antilock` 类在构造时解锁传入的 `Mutex` 对象，并在析构时重新锁定它。通过这种方式，可以安全地在一个作用域内暂时释放锁，执行其他操作。

### 支持 Guard 的扩展

在某些情况下，我们可能不希望 `Antilock` 直接操作互斥锁，而是通过一个管理锁的 `Guard` 对象来间接操作锁。例如，我们可以使用 `std::unique_lock` 作为 Guard，这样可以更加灵活。

下面是一个简单的例子：

```cpp
template<typename Guard>
class Antilock {
public:
Antilock() = default;

    explicit Antilock(Guard& guard)
        : m_mutex(guard.mutex()) {  // 使用 Guard 的 mutex 方法获取互斥锁
        if (m_mutex) {
            m_mutex->unlock();
            std::cout << "[Antilock] Lock released.\n";
        }
    }

    ~Antilock() {
        if (m_mutex) {
            m_mutex->lock();
            std::cout << "[Antilock] Lock reacquired.\n";
        }
    }

private:
    typename Guard::mutex_type* m_mutex = nullptr;
};
```

在这个版本中，`Antilock` 使用 `Guard` 对象的 `mutex()` 方法来获取互斥锁的指针，从而实现对锁的间接管理。

## 使用场景

### 调用外部操作

假设我们有一个共享资源需要在多线程环境下访问，同时我们需要在访问资源的过程中调用一个外部操作。由于外部操作可能会导致长时间等待或死锁，我们希望在执行外部操作时释放锁，操作完成后重新获取锁。
以下是一个使用 `Antilock` 模式的示例：

```cpp
class Resource {
public:
    void DoSomething() {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::cout << "[Resource] Doing something under lock.\n";
        // 临时释放锁，执行外部操作
        Antilock<std::unique_lock<std::mutex>> antilock(lock);
        m_data = ExternalOperation();
        std::cout << "[Resource] Finished doing something.\n";
    }

private:
    int ExternalOperation() {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟耗时操作
        std::cout << "[External] External operation completed.\n";
        return 42; // 返回一个结果
    }

    std::mutex m_mutex;  // 保护共享数据的互斥锁
    int m_data = 0;      // 共享数据
};
```

在这个示例中，`DoSomething` 方法使用 `std::unique_lock` 锁定互斥锁，然后使用 `Antilock` 在执行外部操作时释放锁。外部操作完成后，锁会被自动重新获取。

### 多线程

为了更好地展示 `Antilock` 模式的优势，我们可以创建多个线程同时访问共享资源

```cpp
int main() {
Resource resource;

    // 创建多个线程来访问共享资源
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(&Resource::DoSomething, &resource);
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;

}
```

在这个例子中，我们创建了多个线程，每个线程都会调用 Resource 对象的 DoSomething 方法。Antilock 模式确保了在外部操作期间，锁会被暂时释放，从而避免了可能的死锁。

## 完整例子

[CE链接](https://godbolt.org/z/7nEqT8n5M)

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>
#include <condition_variable>
#include <atomic>
#include <chrono>

// 定义支持 Guard 的 Antilock 模板类
template<typename Guard>
class Antilock {
public:
    Antilock() = default;

    explicit Antilock(Guard& guard)
        : m_mutex(guard.mutex()) {  // 使用 Guard 的 mutex 方法获取互斥锁
        if (m_mutex) {
            m_mutex->unlock();
            std::cout << "[Antilock] Lock released.\n";
        }
    }

    ~Antilock() {
        if (m_mutex) {
            m_mutex->lock();
            std::cout << "[Antilock] Lock reacquired.\n";
        }
    }

private:
    typename Guard::mutex_type* m_mutex = nullptr;  // 指向互斥锁的指针
};

// 模拟一个资源类，包含一个互斥锁和条件变量来保护和协调共享数据
class Resource {
public:
    void DoSomething() {
        std::unique_lock<std::mutex> lock(m_mutex);

        // 使用条件变量等待某些条件满足
        m_condVar.wait(lock, [this] { return m_data.load() == 0; });

        std::cout << "[Resource] Doing something under lock.\n";

        try {
            // 临时释放锁，执行外部操作
            Antilock<std::unique_lock<std::mutex>> antilock(lock);
            m_data.store(ExternalOperation());
        } catch (const std::exception& ex) {
            std::cerr << "[Error] Exception caught: " << ex.what() << "\n";
            // 处理异常的情况下，需要确保锁状态的一致性
        }

        std::cout << "[Resource] Finished doing something.\n";
        m_condVar.notify_all(); // 通知其他等待的线程
    }

private:
    int ExternalOperation() {
        std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟耗时操作
        std::cout << "[External] External operation completed.\n";
        return 42; // 返回一个结果
    }

    std::mutex m_mutex;                  // 保护共享数据的互斥锁
    std::condition_variable m_condVar;   // 用于线程同步的条件变量
    std::atomic<int> m_data{0};          // 线程安全的共享数据
};

// 主函数
int main() {
    Resource resource;

    // 创建多个线程来访问共享资源
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(&Resource::DoSomething, &resource);
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}
```

## 总结

### 优点

- 灵活性：Antilock 模式允许在需要时暂时释放锁，从而执行一些可能导致死锁的操作。
- 自动管理：通过 RAII 模式，Antilock 在作用域结束时自动重新获取锁，无需手动管理锁的状态。

### 注意事项

- 锁状态的复杂性：使用 Antilock 可能会增加锁状态的复杂性，尤其是在嵌套锁定或递归锁定的场景中。
- 异常处理：在实现 Antilock 模式时，确保在出现异常的情况下，锁能够被正确管理，以避免锁定状态不一致的问题。

通过本文的介绍，希望大家能对 Antilock 模式有了更深入的了解，并能够在实际项目中应用这一模式来解决复杂的锁管理问题。如果有问题，欢迎大家在评论区中指出，新人发文，求大佬们轻喷！
