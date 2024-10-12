# Documentation for promise.hpp

This document provides a detailed overview of the `promise.hpp` header file, which contains the `EnhancedPromise` class template for implementing an enhanced version of `std::promise` with additional features.

## Table of Contents

1. [EnhancedPromise Class Template](#enhancedpromise-class-template)
2. [PromiseCancelledException](#promisecancelledexception)
3. [Public Methods](#public-methods)
4. [Specialization for void](#specialization-for-void)
5. [Usage Examples](#usage-examples)

## EnhancedPromise Class Template

The `EnhancedPromise` class template provides an enhanced version of `std::promise` with additional features such as cancellation and completion callbacks.

### Template Parameter

- `T`: The type of the value that the promise will hold.

### Key Features

- Cancellable promises
- Completion callbacks
- Integration with `EnhancedFuture`

## PromiseCancelledException

A custom exception class that is thrown when operations are attempted on a cancelled promise.

```cpp
class PromiseCancelledException : public atom::error::RuntimeError {
public:
    using atom::error::RuntimeError::RuntimeError;
};
```

Two macros are provided for throwing this exception:

- `THROW_PROMISE_CANCELLED_EXCEPTION(...)`
- `THROW_NESTED_PROMISE_CANCELLED_EXCEPTION(...)`

## Public Methods

### Constructor

```cpp
EnhancedPromise();
```

Constructs an `EnhancedPromise` object.

### getEnhancedFuture

```cpp
auto getEnhancedFuture() -> EnhancedFuture<T>;
```

Returns an `EnhancedFuture` associated with this promise.

### setValue

```cpp
void setValue(T value);
```

Sets the value of the promise. Throws `PromiseCancelledException` if the promise has been cancelled.

### setException

```cpp
void setException(std::exception_ptr exception);
```

Sets an exception for the promise. Throws `PromiseCancelledException` if the promise has been cancelled.

### onComplete

```cpp
template <typename F>
void onComplete(F&& func);
```

Adds a callback function to be called when the promise is fulfilled.

### cancel

```cpp
void cancel();
```

Cancels the promise.

### isCancelled

```cpp
[[nodiscard]] auto isCancelled() const -> bool;
```

Checks if the promise has been cancelled.

### getFuture

```cpp
auto getFuture() -> std::shared_future<T>;
```

Returns the underlying `std::shared_future` object.

## Specialization for void

A specialization of `EnhancedPromise` is provided for `void` type. It has similar methods and behavior to the primary template, with appropriate modifications for handling promises without a value.

## Usage Examples

Here are some examples demonstrating how to use the `EnhancedPromise` class:

### Basic Usage

```cpp
#include "promise.hpp"
#include <iostream>

int main() {
    atom::async::EnhancedPromise<int> promise;
    auto future = promise.getEnhancedFuture();

    // Set a value
    promise.setValue(42);

    // Get the result
    std::cout << "Result: " << future.wait() << std::endl;

    return 0;
}
```

### Using Completion Callbacks

```cpp
atom::async::EnhancedPromise<std::string> promise;

promise.onComplete([](const std::string& result) {
    std::cout << "Promise fulfilled with: " << result << std::endl;
});

auto future = promise.getEnhancedFuture();

// Fulfill the promise
promise.setValue("Hello, World!");

// The callback will be called when the promise is fulfilled
future.wait();
```

### Handling Exceptions

```cpp
atom::async::EnhancedPromise<int> promise;
auto future = promise.getEnhancedFuture();

try {
    promise.setException(std::make_exception_ptr(std::runtime_error("Something went wrong")));
} catch (const atom::async::PromiseCancelledException& e) {
    std::cout << "Promise was cancelled: " << e.what() << std::endl;
}

try {
    int result = future.wait();
} catch (const std::exception& e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
}
```

### Cancelling a Promise

```cpp
atom::async::EnhancedPromise<void> promise;
auto future = promise.getEnhancedFuture();

promise.cancel();

try {
    promise.setValue();
} catch (const atom::async::PromiseCancelledException& e) {
    std::cout << "Cannot set value: " << e.what() << std::endl;
}

if (promise.isCancelled()) {
    std::cout << "Promise has been cancelled" << std::endl;
}
```
