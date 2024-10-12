# Documentation for future.hpp

This document provides a detailed overview of the `future.hpp` header file, which contains enhanced future classes and utility functions for asynchronous programming in C++.

## Table of Contents

1. [EnhancedFuture Class](#enhancedfuture-class)
2. [EnhancedFuture<void> Specialization](#enhancedfuturevoid-specialization)
3. [Utility Functions](#utility-functions)
4. [Exception Handling](#exception-handling)
5. [Usage Examples](#usage-examples)

## EnhancedFuture Class

The `EnhancedFuture` class is a template class that wraps a `std::shared_future` and provides additional functionality.

### Template Parameters

- `T`: The type of the value that the future will hold.

### Constructors

```cpp
explicit EnhancedFuture(std::shared_future<T>&& fut);
```

Constructs an `EnhancedFuture` from a `std::shared_future`.

### Methods

#### then

```cpp
template <typename F>
auto then(F&& func);
```

Chains another operation to be executed after the future is completed.

- `func`: A callable that takes the result of the future as an argument.
- Returns: A new `EnhancedFuture` with the result of `func`.

#### waitFor

```cpp
auto waitFor(std::chrono::milliseconds timeout) -> std::optional<T>;
```

Waits for the future to complete with a timeout.

- `timeout`: The maximum time to wait.
- Returns: The result if completed within the timeout, or `std::nullopt` otherwise.

#### isDone

```cpp
[[nodiscard]] auto isDone() const -> bool;
```

Checks if the future has completed.

#### onComplete

```cpp
template <typename F>
void onComplete(F&& func);
```

Sets a callback to be called when the future completes.

- `func`: A callable that takes the result of the future as an argument.

#### wait

```cpp
auto wait() -> T;
```

Waits synchronously for the future to complete and returns the result.

#### cancel

```cpp
void cancel();
```

Cancels the future operation.

#### isCancelled

```cpp
[[nodiscard]] auto isCancelled() const -> bool;
```

Checks if the future has been cancelled.

#### getException

```cpp
auto getException() -> std::exception_ptr;
```

Returns any exception that occurred during the future's execution.

#### retry

```cpp
template <typename F>
auto retry(F&& func, int max_retries);
```

Retries the future operation a specified number of times if it fails.

- `func`: A callable that takes the result of the future as an argument.
- `max_retries`: The maximum number of retry attempts.
- Returns: A new `EnhancedFuture` with the result of the successful retry.

## EnhancedFuture<void> Specialization

A specialization of `EnhancedFuture` for `void` return types. It has similar methods to the primary template, with appropriate modifications for `void` operations.

## Utility Functions

### makeEnhancedFuture

```cpp
template <typename F, typename... Args>
auto makeEnhancedFuture(F&& f, Args&&... args);
```

Creates an `EnhancedFuture` from a function and its arguments.

### whenAll (Iterator Version)

```cpp
template <typename InputIt>
auto whenAll(InputIt first, InputIt last,
             std::optional<std::chrono::milliseconds> timeout = std::nullopt)
    -> std::future<std::vector<typename std::iterator_traits<InputIt>::value_type>>;
```

Waits for all futures in a range to complete.

- `first`, `last`: Iterators defining the range of futures.
- `timeout`: Optional timeout for waiting.
- Returns: A future that resolves to a vector of results from all input futures.

### whenAll (Variadic Template Version)

```cpp
template <typename... Futures>
auto whenAll(Futures&&... futures)
    -> std::future<std::tuple<future_value_t<Futures>...>>;
```

Waits for all given futures to complete.

- `futures`: A variadic pack of futures.
- Returns: A future that resolves to a tuple of results from all input futures.

## Exception Handling

The header defines custom exception types and macros for error handling:

- `InvalidFutureException`: A custom exception for invalid future operations.
- `THROW_INVALID_FUTURE_EXCEPTION`: Macro for throwing `InvalidFutureException`.
- `THROW_NESTED_INVALID_FUTURE_EXCEPTION`: Macro for throwing nested `InvalidFutureException`.

## Usage Examples

Here are some examples demonstrating how to use the `EnhancedFuture` class and utility functions:

```cpp
#include "future.hpp"
#include <iostream>

int main() {
    // Creating an EnhancedFuture
    auto future = atom::async::makeEnhancedFuture([]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 42;
    });

    // Using then() for chaining operations
    auto chainedFuture = future.then([](int value) {
        return value * 2;
    });

    // Waiting with timeout
    auto result = chainedFuture.waitFor(std::chrono::seconds(3));
    if (result) {
        std::cout << "Result: " << *result << std::endl;
    } else {
        std::cout << "Operation timed out" << std::endl;
    }

    // Using whenAll with multiple futures
    auto future1 = atom::async::makeEnhancedFuture([]() { return 1; });
    auto future2 = atom::async::makeEnhancedFuture([]() { return 2; });
    auto future3 = atom::async::makeEnhancedFuture([]() { return 3; });

    auto allFutures = atom::async::whenAll(future1, future2, future3);
    auto [result1, result2, result3] = allFutures.get();

    std::cout << "Results: " << result1 << ", " << result2 << ", " << result3 << std::endl;

    return 0;
}
```

This example demonstrates creating `EnhancedFuture` objects, chaining operations, waiting with timeouts, and using the `whenAll` utility to work with multiple futures concurrently.

Remember to handle exceptions appropriately and consider using the provided exception types and macros for consistent error handling throughout your application.
