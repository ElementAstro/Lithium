/*
 * lock.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-13

Description: Some useful spinlock implementations

**************************************************/

#include "lock.hpp"

namespace atom::async {
void Spinlock::lock() {
    while (flag_.test_and_set(std::memory_order_acquire)) {
        cpu_relax();
    }
}

auto Spinlock::tryLock() -> bool {
    return !flag_.test_and_set(std::memory_order_acquire);
}

void Spinlock::unlock() { flag_.clear(std::memory_order_release); }

auto TicketSpinlock::lock() -> uint64_t {
    const auto TICKET = ticket_.fetch_add(1, std::memory_order_acq_rel);
    while (serving_.load(std::memory_order_acquire) != TICKET) {
        cpu_relax();
    }
    return TICKET;
}

void TicketSpinlock::unlock(uint64_t TICKET) {
    serving_.store(TICKET + 1, std::memory_order_release);
}

void UnfairSpinlock::lock() {
    while (flag_.test_and_set(std::memory_order_acquire)) {
        cpu_relax();
    }
}

void UnfairSpinlock::unlock() { flag_.clear(std::memory_order_release); }
}  // namespace atom::async
