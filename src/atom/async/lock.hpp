/*
 * lock.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-13

Description: Some useful spinlock implementations

**************************************************/

#ifndef ATOM_ASYNC_LOCK_HPP
#define ATOM_ASYNC_LOCK_HPP

#include <atomic>
#include <thread>

namespace Atom::Async
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

        void lock();

        void unlock();
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

        uint64_t lock();

        void unlock(const uint64_t ticket);
    };

    class UnfairSpinlock
    {
        std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

    public:
        UnfairSpinlock() = default;
        UnfairSpinlock(const UnfairSpinlock &) = delete;
        UnfairSpinlock &operator=(const UnfairSpinlock &) = delete;

        void lock();

        void unlock();
    };

    // Scoped lock for Spinlock
    template <typename Mutex>
    class ScopedLock
    {
        Mutex &mutex_;

    public:
        explicit ScopedLock(Mutex &mutex) : mutex_(mutex)
        {
            mutex_.lock();
        }

        ~ScopedLock()
        {
            mutex_.unlock();
        }

        ScopedLock(const ScopedLock &) = delete;
        ScopedLock &operator=(const ScopedLock &) = delete;
    };

    // Scoped lock for TicketSpinlock
    template <typename Mutex>
    class ScopedTicketLock
    {
        Mutex &mutex_;
        const uint64_t ticket_;

    public:
        explicit ScopedTicketLock(Mutex &mutex) : mutex_(mutex), ticket_(mutex_.lock()) {}

        ~ScopedTicketLock()
        {
            mutex_.unlock(ticket_);
        }

        ScopedTicketLock(const ScopedTicketLock &) = delete;
        ScopedTicketLock &operator=(const ScopedTicketLock &) = delete;
    };

    // Scoped lock for UnfairSpinlock
    template <typename Mutex>
    class ScopedUnfairLock
    {
        Mutex &mutex_;

    public:
        explicit ScopedUnfairLock(Mutex &mutex) : mutex_(mutex)
        {
            mutex_.lock();
        }

        ~ScopedUnfairLock()
        {
            mutex_.unlock();
        }

        ScopedUnfairLock(const ScopedUnfairLock &) = delete;
        ScopedUnfairLock &operator=(const ScopedUnfairLock &) = delete;
    };

} // namespace Atom::Async

#endif