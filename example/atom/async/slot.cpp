#include <chrono>
#include <iostream>
#include <thread>

#include "atom/async/limiter.hpp"
#include "atom/async/slot.hpp"

// Example function to be called on signal emission
void exampleHandler(int value) {
    std::cout << "Signal received with value: " << value << " on thread "
              << std::this_thread::get_id() << std::endl;
}

void exampleAsyncHandler(int value) {
    std::cout << "Async signal received with value: " << value << " on thread "
              << std::this_thread::get_id() << std::endl;
}

int main() {
    // Create a signal instance
    atom::async::Signal<int> mySignal;

    // Subscribe to the signal with a handler
    mySignal.connect(exampleHandler);

    // Emit some signals
    for (int i = 0; i < 5; ++i) {
        mySignal.emit(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Create an AsyncSignal instance
    atom::async::AsyncSignal<int> myAsyncSignal;

    // Subscribe to the async signal
    myAsyncSignal.connect(exampleAsyncHandler);

    // Emit some async signals
    for (int i = 5; i < 10; ++i) {
        myAsyncSignal.emit(i);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Demonstrating Debounce
    atom::async::Debounce debouncedSignal(
        []() { std::cout << "Debounced function executed.\n"; },
        std::chrono::milliseconds(500), false);

    // Simulating rapid calls to the debounced function
    std::cout << "Simulating rapid calls to debounced function...\n";
    for (int i = 0; i < 10; ++i) {
        debouncedSignal();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    // Give some time for the debounced function to execute
    std::this_thread::sleep_for(std::chrono::milliseconds(700));

    // Demonstrating Throttle
    atom::async::Throttle throttledSignal(
        []() { std::cout << "Throttled function executed.\n"; },
        std::chrono::milliseconds(1000), true);

    // Simulating rapid calls to the throttled function
    std::cout << "Simulating rapid calls to throttled function...\n";
    for (int i = 0; i < 5; ++i) {
        throttledSignal();
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }

    // Wait some time to ensure throttled function executes
    std::this_thread::sleep_for(std::chrono::milliseconds(1500));

    return 0;
}
