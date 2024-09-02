#include <iostream>
#include <thread>
#include <vector>

#include "atom/async/lock.hpp"

// Global shared variable
int sharedCounter = 0;
const int NUM_INCREMENTS = 1000;

// Example using Spinlock
atom::async::Spinlock spinlock;

void incrementCounterWithSpinlock() {
    for (int i = 0; i < NUM_INCREMENTS; ++i) {
        spinlock.lock();
        ++sharedCounter;  // Critical section
        spinlock.unlock();
    }
}

// Example using TicketSpinlock
atom::async::TicketSpinlock ticketSpinlock;

void incrementCounterWithTicketSpinlock() {
    for (int i = 0; i < NUM_INCREMENTS; ++i) {
        ticketSpinlock.lock();
        ++sharedCounter;  // Critical section
        ticketSpinlock.unlock(
            0);  // Unlock with ticket 0 (not optimal for brevity)
    }
}

// Example using UnfairSpinlock
atom::async::UnfairSpinlock unfairSpinlock;

void incrementCounterWithUnfairSpinlock() {
    for (int i = 0; i < NUM_INCREMENTS; ++i) {
        unfairSpinlock.lock();
        ++sharedCounter;  // Critical section
        unfairSpinlock.unlock();
    }
}

int main() {
    sharedCounter = 0;  // Reset shared counter

    // Using Spinlock
    std::vector<std::thread> threads;
    std::cout << "Using Spinlock:\n";
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(incrementCounterWithSpinlock);
    }
    for (auto &t : threads) {
        t.join();
    }
    std::cout << "Final counter value (Spinlock): " << sharedCounter << "\n";

    // Reset shared counter for next demo
    sharedCounter = 0;
    threads.clear();

    // Using TicketSpinlock
    std::cout << "Using TicketSpinlock:\n";
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(incrementCounterWithTicketSpinlock);
    }
    for (auto &t : threads) {
        t.join();
    }
    std::cout << "Final counter value (TicketSpinlock): " << sharedCounter
              << "\n";

    // Reset shared counter for next demo
    sharedCounter = 0;
    threads.clear();

    // Using UnfairSpinlock
    std::cout << "Using UnfairSpinlock:\n";
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(incrementCounterWithUnfairSpinlock);
    }
    for (auto &t : threads) {
        t.join();
    }
    std::cout << "Final counter value (UnfairSpinlock): " << sharedCounter
              << "\n";

    return 0;
}
