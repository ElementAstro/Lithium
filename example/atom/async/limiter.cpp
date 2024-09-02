#include <chrono>
#include <iostream>
#include <thread>

#include "atom/async/limiter.hpp"

// Function to be rate limited
void criticalFunction() {
    std::cout << "Critical function executed at "
              << std::chrono::steady_clock::now().time_since_epoch().count()
              << std::endl;
}

// Function to demonstrate debouncing
void debouncedFunction() {
    std::cout << "Debounced function executed at "
              << std::chrono::steady_clock::now().time_since_epoch().count()
              << std::endl;
}

// Function to demonstrate throttling
void throttledFunction() {
    std::cout << "Throttled function executed at "
              << std::chrono::steady_clock::now().time_since_epoch().count()
              << std::endl;
}

int main() {
    // Rate Limiter Example
    atom::async::RateLimiter rateLimiter;
    rateLimiter.setFunctionLimit("criticalFunction", 3,
                                 std::chrono::seconds(5));

    // Simulate requests to the critical function
    for (int i = 0; i < 5; ++i) {
        auto awaiter = rateLimiter.acquire("criticalFunction");
        awaiter.await_suspend({});
        criticalFunction();
        std::this_thread::sleep_for(
            std::chrono::seconds(1));  // Simulate time between function calls
    }

    // Debounce Example
    atom::async::Debounce debouncer(debouncedFunction,
                                    std::chrono::milliseconds(500), true);

    // Simulate rapid calls
    for (int i = 0; i < 5; ++i) {
        debouncer();  // Calls will be debounced
        std::this_thread::sleep_for(
            std::chrono::milliseconds(200));  // Calls within the debounce delay
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(600));  // Wait for debounced call to execute

    // Throttle Example
    atom::async::Throttle throttler(throttledFunction,
                                    std::chrono::milliseconds(1000), true);

    // Simulate rapid throttled calls
    for (int i = 0; i < 5; ++i) {
        throttler();  // Throttled function calls
        std::this_thread::sleep_for(
            std::chrono::milliseconds(300));  // Calls within the throttle time
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(2000));  // Wait to ensure throttling works

    return 0;
}
