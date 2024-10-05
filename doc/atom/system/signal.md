# Signal Handling Library Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [SignalHandlerRegistry](#signalhandlerregistry)
   - [Overview](#overview)
   - [Key Methods](#key-methods)
   - [Usage Examples](#usage-examples)
3. [SafeSignalManager](#safesignalmanager)
   - [Overview](#overview-1)
   - [Key Methods](#key-methods-1)
   - [Usage Examples](#usage-examples-1)
4. [Best Practices](#best-practices)

## Introduction

This library provides two main classes for handling signals in C++ applications: `SignalHandlerRegistry` and `SafeSignalManager`. These classes offer different approaches to managing signal handlers, with `SafeSignalManager` providing additional thread safety features.

## SignalHandlerRegistry

### Overview

`SignalHandlerRegistry` is a singleton class that manages signal handlers with priorities. It allows you to register multiple handlers for each signal and execute them in order of priority.

### Key Methods

1. `static SignalHandlerRegistry& getInstance()`

   - Returns the singleton instance of `SignalHandlerRegistry`.

2. `void setSignalHandler(SignalID signal, const SignalHandler& handler, int priority = 0)`

   - Registers a signal handler for a specific signal with an optional priority.

3. `void removeSignalHandler(SignalID signal, const SignalHandler& handler)`

   - Removes a specific signal handler for a signal.

4. `void setStandardCrashHandlerSignals(const SignalHandler& handler, int priority = 0)`
   - Sets handlers for standard crash signals.

### Usage Examples

#### Basic Usage

```cpp
#include "signal_handler.h"
#include <iostream>

void customSigIntHandler(SignalID signal) {
    std::cout << "Received SIGINT. Performing custom action." << std::endl;
    // Perform cleanup or custom actions
    exit(signal);
}

int main() {
    auto& registry = SignalHandlerRegistry::getInstance();

    // Register a custom SIGINT handler
    registry.setSignalHandler(SIGINT, customSigIntHandler);

    // Your main program logic here
    while (true) {
        // Do some work
    }

    return 0;
}
```

#### Using Priorities

```cpp
#include "signal_handler.h"
#include <iostream>

void highPriorityHandler(SignalID signal) {
    std::cout << "High priority handler for signal " << signal << std::endl;
}

void lowPriorityHandler(SignalID signal) {
    std::cout << "Low priority handler for signal " << signal << std::endl;
}

int main() {
    auto& registry = SignalHandlerRegistry::getInstance();

    // Register handlers with different priorities
    registry.setSignalHandler(SIGTERM, highPriorityHandler, 10);
    registry.setSignalHandler(SIGTERM, lowPriorityHandler, 1);

    // The high priority handler will be called first when SIGTERM is received

    // Your main program logic here

    return 0;
}
```

#### Setting Standard Crash Handlers

```cpp
#include "signal_handler.h"
#include <iostream>

void crashHandler(SignalID signal) {
    std::cerr << "Crash detected! Signal: " << signal << std::endl;
    // Perform crash logging or cleanup
    exit(signal);
}

int main() {
    auto& registry = SignalHandlerRegistry::getInstance();

    // Set up handlers for standard crash signals
    registry.setStandardCrashHandlerSignals(crashHandler);

    // Your main program logic here

    return 0;
}
```

## SafeSignalManager

### Overview

`SafeSignalManager` provides a thread-safe approach to signal handling. It processes signals in a separate thread to avoid blocking the main execution and ensure safe handling of signals.

### Key Methods

1. `static SafeSignalManager& getInstance()`

   - Returns the singleton instance of `SafeSignalManager`.

2. `void addSafeSignalHandler(SignalID signal, const SignalHandler& handler, int priority = 0)`

   - Adds a signal handler for a specific signal with an optional priority.

3. `void removeSafeSignalHandler(SignalID signal, const SignalHandler& handler)`

   - Removes a specific signal handler for a signal.

4. `void clearSignalQueue()`

   - Clears the queue of pending signals.

5. `static void safeSignalDispatcher(int signal)`
   - Static method to safely dispatch signals to the manager.

### Usage Examples

#### Basic Usage

```cpp
#include "safe_signal_manager.h"
#include <iostream>

void safeSignalHandler(SignalID signal) {
    std::cout << "Safely handling signal: " << signal << std::endl;
}

int main() {
    auto& manager = SafeSignalManager::getInstance();

    // Add a safe signal handler for SIGINT
    manager.addSafeSignalHandler(SIGINT, safeSignalHandler);

    // Set up the system signal handler to use SafeSignalManager
    std::signal(SIGINT, SafeSignalManager::safeSignalDispatcher);

    // Your main program logic here
    while (true) {
        // Do some work
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}
```

#### Using Multiple Handlers with Priorities

```cpp
#include "safe_signal_manager.h"
#include <iostream>

void highPriorityHandler(SignalID signal) {
    std::cout << "High priority handler for signal " << signal << std::endl;
}

void lowPriorityHandler(SignalID signal) {
    std::cout << "Low priority handler for signal " << signal << std::endl;
}

int main() {
    auto& manager = SafeSignalManager::getInstance();

    // Add handlers with different priorities
    manager.addSafeSignalHandler(SIGTERM, highPriorityHandler, 10);
    manager.addSafeSignalHandler(SIGTERM, lowPriorityHandler, 1);

    // Set up the system signal handler
    std::signal(SIGTERM, SafeSignalManager::safeSignalDispatcher);

    // Your main program logic here

    return 0;
}
```

#### Clearing Signal Queue

```cpp
#include "safe_signal_manager.h"
#include <iostream>

void signalHandler(SignalID signal) {
    std::cout << "Handling signal: " << signal << std::endl;
}

int main() {
    auto& manager = SafeSignalManager::getInstance();

    manager.addSafeSignalHandler(SIGUSR1, signalHandler);

    // Set up the system signal handler
    std::signal(SIGUSR1, SafeSignalManager::safeSignalDispatcher);

    // Your main program logic here

    // If you need to clear pending signals:
    manager.clearSignalQueue();

    return 0;
}
```

## Best Practices

1. **Use SafeSignalManager for Thread-Safe Applications**: If your application is multi-threaded, prefer `SafeSignalManager` over `SignalHandlerRegistry` to ensure thread-safe signal handling.

2. **Prioritize Handlers**: Use the priority parameter when setting handlers to ensure critical actions are performed first.

3. **Keep Signal Handlers Short**: Signal handlers should be quick and non-blocking. For complex operations, set a flag or use a queue to handle the action in the main program flow.

4. **Be Cautious with Resource Allocation**: Avoid allocating memory or using non-reentrant functions within signal handlers.
