# Trigger Class Documentation

## Overview

The `Trigger` class is a templated class designed for handling event-driven callbacks with parameter support. It is defined in the `atom::async` namespace and is part of the `trigger.hpp` header file. This class allows users to register, unregister, and trigger callbacks for different events, providing a mechanism to manage callbacks with priorities and delays.

## Class Declaration

```cpp
namespace atom::async {
template <typename ParamType>
    requires CallableWithParam<ParamType>
class Trigger {
    // ... (member functions and private members)
};
}
```

The `Trigger` class is templated with `ParamType`, which must satisfy the `CallableWithParam` concept.

## Concepts

### CallableWithParam

```cpp
template <typename ParamType>
concept CallableWithParam = requires(ParamType p) {
    std::invoke(std::declval<std::function<void(ParamType)>>(), p);
};
```

This concept checks if a `std::function` taking a parameter of type `ParamType` is invocable with an instance of `ParamType`.

## Type Aliases and Enums

```cpp
using Callback = std::function<void(ParamType)>;

enum class CallbackPriority { High, Normal, Low };
```

- `Callback`: A type alias for the callback function.
- `CallbackPriority`: An enumeration for callback priority levels.

## Member Functions

### registerCallback

```cpp
void registerCallback(const std::string& event, Callback callback,
                      CallbackPriority priority = CallbackPriority::Normal);
```

Registers a callback for a specified event.

- **Parameters:**
  - `event`: The name of the event for which the callback is registered.
  - `callback`: The callback function to be executed when the event is triggered.
  - `priority`: The priority level of the callback (default is Normal).

### unregisterCallback

```cpp
void unregisterCallback(const std::string& event, Callback callback);
```

Unregisters a callback for a specified event.

- **Parameters:**
  - `event`: The name of the event from which the callback is unregistered.
  - `callback`: The callback function to be removed.

### trigger

```cpp
void trigger(const std::string& event, const ParamType& param);
```

Triggers the callbacks associated with a specified event.

- **Parameters:**
  - `event`: The name of the event to trigger.
  - `param`: The parameter to be passed to the callbacks.

### scheduleTrigger

```cpp
void scheduleTrigger(const std::string& event, const ParamType& param,
                     std::chrono::milliseconds delay);
```

Schedules a trigger for a specified event after a delay.

- **Parameters:**
  - `event`: The name of the event to trigger.
  - `param`: The parameter to be passed to the callbacks.
  - `delay`: The delay after which to trigger the event, specified in milliseconds.

### scheduleAsyncTrigger

```cpp
auto scheduleAsyncTrigger(const std::string& event,
                          const ParamType& param) -> std::future<void>;
```

Schedules an asynchronous trigger for a specified event.

- **Parameters:**
  - `event`: The name of the event to trigger.
  - `param`: The parameter to be passed to the callbacks.
- **Returns:** A future representing the ongoing operation to trigger the event.

### cancelTrigger

```cpp
void cancelTrigger(const std::string& event);
```

Cancels the scheduled trigger for a specified event.

- **Parameters:**
  - `event`: The name of the event for which to cancel the trigger.

### cancelAllTriggers

```cpp
void cancelAllTriggers();
```

Cancels all scheduled triggers.

## Usage Example

Here's a simple example demonstrating how to use the `Trigger` class:

```cpp
#include "trigger.hpp"
#include <iostream>
#include <string>

int main() {
    atom::async::Trigger<std::string> eventTrigger;

    // Register callbacks
    eventTrigger.registerCallback("greet", [](const std::string& name) {
        std::cout << "Hello, " << name << "!" << std::endl;
    }, atom::async::Trigger<std::string>::CallbackPriority::High);

    eventTrigger.registerCallback("greet", [](const std::string& name) {
        std::cout << "Nice to meet you, " << name << "." << std::endl;
    }, atom::async::Trigger<std::string>::CallbackPriority::Normal);

    // Trigger the event
    eventTrigger.trigger("greet", "Alice");

    // Schedule a trigger
    eventTrigger.scheduleTrigger("greet", "Bob", std::chrono::seconds(2));

    // Wait for scheduled trigger
    std::this_thread::sleep_for(std::chrono::seconds(3));

    return 0;
}
```

This example creates a `Trigger` object for events with `std::string` parameters, registers two callbacks for the "greet" event with different priorities, triggers the event immediately, and then schedules another trigger after a 2-second delay.

## Notes

- The `Trigger` class is thread-safe, using a mutex to protect access to its internal structures.
- Callbacks are sorted by priority before execution, with higher priority callbacks executed first.
- Exceptions thrown by callbacks are caught and swallowed to prevent them from interrupting the execution of other callbacks.
- The class supports both synchronous and asynchronous triggering of events.
- Users should be careful when using `cancelTrigger` and `cancelAllTriggers`, as they will remove all registered callbacks for the specified event(s).
