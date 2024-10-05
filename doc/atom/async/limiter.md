# Documentation for limiter.hpp

This document provides a detailed overview of the `limiter.hpp` header file, which contains classes for implementing rate limiting, debouncing, and throttling in C++.

## Table of Contents

1. [RateLimiter Class](#ratelimiter-class)
2. [Debounce Class](#debounce-class)
3. [Throttle Class](#throttle-class)
4. [Usage Examples](#usage-examples)

## RateLimiter Class

The `RateLimiter` class is designed to control the rate of function executions.

### Nested Classes and Structures

#### Settings Structure

```cpp
struct Settings {
    size_t maxRequests;
    std::chrono::seconds timeWindow;

    explicit Settings(size_t max_requests = 5, std::chrono::seconds time_window = std::chrono::seconds(1));
};
```

Defines the settings for the rate limiter, including the maximum number of requests allowed in a given time window.

#### Awaiter Class

```cpp
class Awaiter {
public:
    Awaiter(RateLimiter& limiter, const std::string& function_name);
    auto await_ready() -> bool;
    void await_suspend(std::coroutine_handle<> handle);
    void await_resume();
};
```

An awaiter class for handling coroutines in the rate limiter.

### Public Methods

```cpp
RateLimiter();
Awaiter acquire(const std::string& function_name);
void setFunctionLimit(const std::string& function_name, size_t max_requests, std::chrono::seconds time_window);
void pause();
void resume();
void printLog();
auto getRejectedRequests(const std::string& function_name) -> size_t;
```

- `acquire`: Acquires the rate limiter for a specific function.
- `setFunctionLimit`: Sets the rate limit for a specific function.
- `pause`: Pauses the rate limiter.
- `resume`: Resumes the rate limiter.
- `printLog`: Prints the log of requests.
- `getRejectedRequests`: Gets the number of rejected requests for a specific function.

### Private Methods

```cpp
void cleanup(const std::string& function_name, const std::chrono::seconds& time_window);
void processWaiters();
```

- `cleanup`: Cleans up old requests outside the time window.
- `processWaiters`: Processes waiting coroutines.

## Debounce Class

The `Debounce` class implements a debouncing mechanism for function calls.

### Constructor

```cpp
Debounce(std::function<void()> func, std::chrono::milliseconds delay, bool leading = false, std::optional<std::chrono::milliseconds> maxWait = std::nullopt);
```

Creates a `Debounce` object with the specified function, delay, leading flag, and optional maximum wait time.

### Public Methods

```cpp
void operator()();
void cancel();
void flush();
void reset();
size_t callCount() const;
```

- `operator()`: Invokes the debounced function if the delay has elapsed since the last call.
- `cancel`: Cancels any pending function calls.
- `flush`: Immediately invokes the function if it is scheduled to be called.
- `reset`: Resets the debouncer, clearing any pending function call and timer.
- `callCount`: Returns the number of times the function has been invoked.

### Private Method

```cpp
void run();
```

Runs the function in a separate thread after the debounce delay.

## Throttle Class

The `Throttle` class provides throttling for function calls, ensuring they are not invoked more frequently than a specified interval.

### Constructor

```cpp
Throttle(std::function<void()> func, std::chrono::milliseconds interval, bool leading = false, std::optional<std::chrono::milliseconds> maxWait = std::nullopt);
```

Creates a `Throttle` object with the specified function, interval, leading flag, and optional maximum wait time.

### Public Methods

```cpp
void operator()();
void cancel();
void reset();
auto callCount() const -> size_t;
```

- `operator()`: Invokes the throttled function if the interval has elapsed.
- `cancel`: Cancels any pending function calls.
- `reset`: Resets the throttle, allowing the function to be invoked immediately if required.
- `callCount`: Returns the number of times the function has been called.

## Usage Examples

Here are some examples demonstrating how to use the classes in this header:

### RateLimiter Example

```cpp
#include "limiter.hpp"
#include <iostream>
#include <thread>

int main() {
    atom::async::RateLimiter limiter;

    // Set a limit for a specific function
    limiter.setFunctionLimit("myFunction", 5, std::chrono::seconds(10));

    // Use the rate limiter in a coroutine
    auto myCoroutine = [&]() -> std::future<void> {
        co_await limiter.acquire("myFunction");
        std::cout << "Function executed" << std::endl;
    };

    // Call the coroutine multiple times
    for (int i = 0; i < 10; ++i) {
        myCoroutine();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Print the log and rejected requests
    limiter.printLog();
    std::cout << "Rejected requests: " << limiter.getRejectedRequests("myFunction") << std::endl;

    return 0;
}
```

### Debounce Example

```cpp
#include "limiter.hpp"
#include <iostream>
#include <thread>

int main() {
    atom::async::Debounce debouncer(
        []() { std::cout << "Debounced function called" << std::endl; },
        std::chrono::milliseconds(500)
    );

    // Call the debounced function multiple times
    for (int i = 0; i < 5; ++i) {
        debouncer();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Wait to see the debounced call
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Function called " << debouncer.callCount() << " times" << std::endl;

    return 0;
}
```

### Throttle Example

```cpp
#include "limiter.hpp"
#include <iostream>
#include <thread>

int main() {
    atom::async::Throttle throttle(
        []() { std::cout << "Throttled function called" << std::endl; },
        std::chrono::milliseconds(1000)
    );

    // Call the throttled function multiple times
    for (int i = 0; i < 5; ++i) {
        throttle();
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // Wait to see all throttled calls
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::cout << "Function called " << throttle.callCount() << " times" << std::endl;

    return 0;
}
```

These examples demonstrate basic usage of the `RateLimiter`, `Debounce`, and `Throttle` classes. Remember to handle exceptions and edge cases in your actual implementations.
