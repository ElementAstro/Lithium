# Documentation for eventstack.hpp

This document provides a detailed overview of the `eventstack.hpp` header file, which contains the `EventStack` class template for managing events in a thread-safe manner.

## Table of Contents

1. [EventStack Class Template](#eventstack-class-template)
2. [Constructor and Rule of Five](#constructor-and-rule-of-five)
3. [Public Methods](#public-methods)
4. [Private Members](#private-members)
5. [Usage Examples](#usage-examples)

## EventStack Class Template

The `EventStack` class is a thread-safe stack data structure for managing events. It is implemented as a template class, allowing it to work with any event type.

```cpp
template <typename T>
class EventStack {
    // ...
};
```

## Constructor and Rule of Five

The `EventStack` class follows the Rule of Five, explicitly defining the following special member functions:

1. Default constructor
2. Destructor
3. Copy constructor
4. Copy assignment operator
5. Move constructor
6. Move assignment operator

### Copy Constructor

```cpp
EventStack(const EventStack& other);
```

Creates a new `EventStack` by copying the contents of another `EventStack`.

### Copy Assignment Operator

```cpp
EventStack& operator=(const EventStack& other);
```

Assigns the contents of another `EventStack` to this `EventStack`.

### Move Constructor

```cpp
EventStack(EventStack&& other) noexcept;
```

Creates a new `EventStack` by moving the contents of another `EventStack`.

### Move Assignment Operator

```cpp
EventStack& operator=(EventStack&& other) noexcept;
```

Moves the contents of another `EventStack` to this `EventStack`.

## Public Methods

### pushEvent

```cpp
void pushEvent(T event);
```

Pushes an event onto the stack.

### popEvent

```cpp
auto popEvent() -> std::optional<T>;
```

Pops an event from the stack. Returns `std::nullopt` if the stack is empty.

### printEvents

```cpp
void printEvents() const;
```

Prints all events in the stack. Only available when `ENABLE_DEBUG` is defined.

### isEmpty

```cpp
auto isEmpty() const -> bool;
```

Checks if the stack is empty.

### size

```cpp
auto size() const -> size_t;
```

Returns the number of events in the stack.

### clearEvents

```cpp
void clearEvents();
```

Clears all events from the stack.

### peekTopEvent

```cpp
auto peekTopEvent() const -> std::optional<T>;
```

Returns the top event in the stack without removing it. Returns `std::nullopt` if the stack is empty.

### copyStack

```cpp
auto copyStack() const -> EventStack<T>;
```

Creates and returns a copy of the current stack.

### filterEvents

```cpp
void filterEvents(std::function<bool(const T&)> filterFunc);
```

Filters events based on a custom filter function.

### serializeStack

```cpp
auto serializeStack() const -> std::string;
```

Serializes the stack into a string.

### deserializeStack

```cpp
void deserializeStack(std::string_view serializedData);
```

Deserializes a string into the stack.

### removeDuplicates

```cpp
void removeDuplicates();
```

Removes duplicate events from the stack.

### sortEvents

```cpp
void sortEvents(std::function<bool(const T&, const T&)> compareFunc);
```

Sorts the events in the stack based on a custom comparison function.

### reverseEvents

```cpp
void reverseEvents();
```

Reverses the order of events in the stack.

### countEvents

```cpp
auto countEvents(std::function<bool(const T&)> predicate) const -> size_t;
```

Counts the number of events that satisfy a predicate.

### findEvent

```cpp
auto findEvent(std::function<bool(const T&)> predicate) const -> std::optional<T>;
```

Finds the first event that satisfies a predicate. Returns `std::nullopt` if not found.

### anyEvent

```cpp
auto anyEvent(std::function<bool(const T&)> predicate) const -> bool;
```

Checks if any event in the stack satisfies a predicate.

### allEvents

```cpp
auto allEvents(std::function<bool(const T&)> predicate) const -> bool;
```

Checks if all events in the stack satisfy a predicate.

## Private Members

- `std::vector<T> events_`: Vector to store events.
- `mutable std::shared_mutex mtx_`: Mutex for thread safety.
- `std::atomic<size_t> eventCount_`: Atomic counter for event count.

## Usage Examples

Here are some examples of how to use the `EventStack` class:

```cpp
#include "eventstack.hpp"
#include <iostream>
#include <string>

int main() {
    atom::async::EventStack<std::string> eventStack;

    // Pushing events
    eventStack.pushEvent("Event 1");
    eventStack.pushEvent("Event 2");
    eventStack.pushEvent("Event 3");

    // Popping events
    auto event = eventStack.popEvent();
    if (event) {
        std::cout << "Popped event: " << *event << std::endl;
    }

    // Checking size and emptiness
    std::cout << "Stack size: " << eventStack.size() << std::endl;
    std::cout << "Is stack empty? " << (eventStack.isEmpty() ? "Yes" : "No") << std::endl;

    // Peeking at the top event
    auto topEvent = eventStack.peekTopEvent();
    if (topEvent) {
        std::cout << "Top event: " << *topEvent << std::endl;
    }

    // Filtering events
    eventStack.filterEvents([](const std::string& event) {
        return event.length() > 5;
    });

    // Serializing and deserializing
    std::string serialized = eventStack.serializeStack();
    atom::async::EventStack<std::string> newStack;
    newStack.deserializeStack(serialized);

    // Sorting events
    newStack.sortEvents([](const std::string& a, const std::string& b) {
        return a < b;
    });

    // Counting events
    size_t count = newStack.countEvents([](const std::string& event) {
        return event.starts_with("Event");
    });
    std::cout << "Number of events starting with 'Event': " << count << std::endl;

    return 0;
}
```

This example demonstrates various operations on the `EventStack`, including pushing and popping events, checking the stack's properties, filtering, serializing, deserializing, sorting, and counting events.

Remember that the `EventStack` is thread-safe, so you can use it in multi-threaded applications without additional synchronization.
