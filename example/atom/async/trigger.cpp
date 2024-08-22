#include <iostream>

#include "atom/async/trigger.hpp"

int main() {
    atom::async::Trigger<int> trigger;

    // Registering callbacks
    trigger.registerCallback(
        "onEvent", [](int x) { std::cout << "Callback 1: " << x << std::endl; },
        atom::async::Trigger<int>::CallbackPriority::High);
    trigger.registerCallback("onEvent", [](int x) {
        std::cout << "Callback 2: " << x << std::endl;
    });

    // Triggering event
    trigger.trigger("onEvent", 42);

    // Scheduling a delayed trigger
    trigger.scheduleTrigger("onEvent", 84, std::chrono::milliseconds(500));

    // Scheduling async trigger
    auto future = trigger.scheduleAsyncTrigger("onEvent", 126);
    future.get();  // Waiting for async trigger to complete

    // Cancel an event
    trigger.cancelTrigger("onEvent");

    // Cancel all events
    trigger.cancelAllTriggers();
}