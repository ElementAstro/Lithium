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

namespace Atom::Async {
void Spinlock::lock() {
    while (flag_.test_and_set(std::memory_order_acquire)) {
        cpu_relax();
    }
}

void Spinlock::unlock() { flag_.clear(std::memory_order_release); }

uint64_t TicketSpinlock::lock() {
    const auto ticket = ticket_.fetch_add(1, std::memory_order_acq_rel);
    while (serving_.load(std::memory_order_acquire) != ticket) {
        cpu_relax();
    }
    return ticket;
}

void TicketSpinlock::unlock(const uint64_t ticket) {
    serving_.store(ticket + 1, std::memory_order_release);
}

void UnfairSpinlock::lock() {
    while (flag_.test_and_set(std::memory_order_acquire)) {
        cpu_relax();
    }
}

void UnfairSpinlock::unlock() { flag_.clear(std::memory_order_release); }
}  // namespace Atom::Async