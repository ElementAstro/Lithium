#pragma once

#include <atomic>
#include <thread>

namespace Atom::Utils
{

// Pause instruction to prevent excess processor bus usage
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

    class Spinlock
    {
        std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

    public:
        Spinlock() = default;
        Spinlock(const Spinlock &) = delete;
        Spinlock &operator=(const Spinlock &) = delete;

        void lock()
        {
            while (flag_.test_and_set(std::memory_order_acquire))
            {
                cpu_relax();
            }
        }

        void unlock()
        {
            flag_.clear(std::memory_order_release);
        }
    };

    class TicketSpinlock
    {
        std::atomic<uint64_t> ticket_{0};
        std::atomic<uint64_t> serving_{0};

    public:
        TicketSpinlock() = default;
        TicketSpinlock(const TicketSpinlock &) = delete;
        TicketSpinlock &operator=(const TicketSpinlock &) = delete;

        class LockGuard
        {
            TicketSpinlock &spinlock_;
            const uint64_t ticket_;

        public:
            LockGuard(TicketSpinlock &spinlock) : spinlock_(spinlock), ticket_(spinlock_.lock()) {}
            ~LockGuard() { spinlock_.unlock(ticket_); }
        };

        using scoped_lock = LockGuard;

        uint64_t lock()
        {
            const auto ticket = ticket_.fetch_add(1, std::memory_order_acq_rel);
            while (serving_.load(std::memory_order_acquire) != ticket)
            {
                cpu_relax();
            }
            return ticket;
        }

        void unlock(const uint64_t ticket)
        {
            serving_.store(ticket + 1, std::memory_order_release);
        }
    };

    class UnfairSpinlock
    {
        std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

    public:
        UnfairSpinlock() = default;
        UnfairSpinlock(const UnfairSpinlock &) = delete;
        UnfairSpinlock &operator=(const UnfairSpinlock &) = delete;

        void lock()
        {
            while (flag_.test_and_set(std::memory_order_acquire))
            {
                cpu_relax();
            }
        }

        void unlock()
        {
            flag_.clear(std::memory_order_release);
        }
    };

} // namespace atomic_queue

/*

Spinlock spinlock;
TicketSpinlock ticketSpinlock;
UnfairSpinlock unfairSpinlock;

void SpinlockExample()
{
    std::cout << "Thread " << std::this_thread::get_id() << " is trying to acquire Spinlock" << std::endl;
    spinlock.lock();
    std::cout << "Thread " << std::this_thread::get_id() << " acquired Spinlock" << std::endl;

    // 临界区代码

    spinlock.unlock();
    std::cout << "Thread " << std::this_thread::get_id() << " released Spinlock" << std::endl;
}

void TicketSpinlockExample()
{
    std::cout << "Thread " << std::this_thread::get_id() << " is trying to acquire TicketSpinlock" << std::endl;
    auto ticket = ticketSpinlock.lock();
    std::cout << "Thread " << std::this_thread::get_id() << " acquired TicketSpinlock" << std::endl;

    // 临界区代码

    ticketSpinlock.unlock(ticket);
    std::cout << "Thread " << std::this_thread::get_id() << " released TicketSpinlock" << std::endl;
}

void UnfairSpinlockExample()
{
    std::cout << "Thread " << std::this_thread::get_id() << " is trying to acquire UnfairSpinlock" << std::endl;
    unfairSpinlock.lock();
    std::cout << "Thread " << std::this_thread::get_id() << " acquired UnfairSpinlock" << std::endl;

    // 临界区代码

    unfairSpinlock.unlock();
    std::cout << "Thread " << std::this_thread::get_id() << " released UnfairSpinlock" << std::endl;
}

int main()
{
    // 使用 Spinlock
    std::thread t1(SpinlockExample);
    std::thread t2(SpinlockExample);

    t1.join();
    t2.join();

    std::cout << std::endl;

    // 使用 TicketSpinlock
    std::thread t3(TicketSpinlockExample);
    std::thread t4(TicketSpinlockExample);

    t3.join();
    t4.join();

    std::cout << std::endl;

    // 使用 UnfairSpinlock
    std::thread t5(UnfairSpinlockExample);
    std::thread t6(UnfairSpinlockExample);

    t5.join();
    t6.join();

    return 0;
}

*/
