# Thread Wrapper Documentation

## Overview

The `Thread` class is a wrapper for managing C++20 `std::jthread` objects, providing a convenient interface for starting, stopping, and joining threads. It is defined in the `atom::async` namespace and is part of the `thread_wrapper.hpp` header file.

## Class Declaration

```cpp
namespace atom::async {
class Thread : public NonCopyable {
    // ... (member functions and private members)
};
}
```

The `Thread` class inherits from `NonCopyable`, indicating that objects of this class cannot be copied.

## Constructor

```cpp
Thread() = default;
```

The class uses the default constructor.

## Member Functions

### start

```cpp
template <typename Callable, typename... Args>
void start(Callable&& func, Args&&... args);
```

Starts a new thread with the specified callable object and arguments.

- **Parameters:**

  - `func`: The callable object to execute in the new thread.
  - `args`: The arguments to pass to the callable object.

- **Behavior:**
  - If `func` is invocable with a `std::stop_token` and the provided arguments, it will be invoked with a `std::stop_token` as the first argument.
  - Otherwise, it will be invoked with only the provided arguments.

### requestStop

```cpp
void requestStop();
```

Requests the thread to stop execution.

### join

```cpp
void join();
```

Waits for the thread to finish execution.

### running

```cpp
[[nodiscard]] auto running() const noexcept -> bool;
```

Checks if the thread is currently running.

- **Returns:** `true` if the thread is running, `false` otherwise.

### swap

```cpp
void swap(Thread& other) noexcept;
```

Swaps the content of this `Thread` object with another `Thread` object.

- **Parameters:**
  - `other`: The `Thread` object to swap with.

### getThread

```cpp
[[nodiscard]] auto getThread() noexcept -> std::jthread&;
[[nodiscard]] auto getThread() const noexcept -> const std::jthread&;
```

Gets the underlying `std::jthread` object.

- **Returns:** Reference to the underlying `std::jthread` object.

### getId

```cpp
[[nodiscard]] auto getId() const noexcept -> std::thread::id;
```

Gets the ID of the thread.

- **Returns:** The ID of the thread.

### getStopSource

```cpp
[[nodiscard]] auto getStopSource() noexcept -> std::stop_source;
```

Gets the underlying `std::stop_source` object.

- **Returns:** The underlying `std::stop_source` object.

### getStopToken

```cpp
[[nodiscard]] auto getStopToken() const noexcept -> std::stop_token;
```

Gets the underlying `std::stop_token` object.

- **Returns:** The underlying `std::stop_token` object.

## Usage Example

Here's a simple example demonstrating how to use the `Thread` class:

```cpp
#include "thread_wrapper.hpp"
#include <iostream>
#include <chrono>

void exampleFunction(std::stop_token stoken, int duration) {
    while (!stoken.stop_requested()) {
        std::cout << "Thread running..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        duration--;
        if (duration <= 0) break;
    }
    std::cout << "Thread finished." << std::endl;
}

int main() {
    atom::async::Thread myThread;
    myThread.start(exampleFunction, 5);

    std::this_thread::sleep_for(std::chrono::seconds(3));
    myThread.requestStop();
    myThread.join();

    return 0;
}
```

This example creates a `Thread` object, starts a new thread that runs for a specified duration or until stopped, waits for 3 seconds, requests the thread to stop, and then joins the thread.

## Notes

- The `Thread` class is designed to work with C++20 features, particularly `std::jthread`.
- It provides a higher-level interface for thread management compared to using `std::jthread` directly.
- The class is non-copyable but can be moved (swap operation is provided).
- It's important to call `join()` or ensure the thread has finished before the `Thread` object is destroyed to avoid potential issues.
