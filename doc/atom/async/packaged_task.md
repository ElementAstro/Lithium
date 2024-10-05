# Documentation for packaged_task.hpp

This document provides a detailed overview of the `packaged_task.hpp` header file, which contains the `EnhancedPackagedTask` class template for implementing an enhanced version of `std::packaged_task` with additional features.

## Table of Contents

1. [EnhancedPackagedTask Class Template](#enhancedpackagedtask-class-template)
2. [Constructor](#constructor)
3. [Public Methods](#public-methods)
4. [Protected Members](#protected-members)
5. [Specialization for void Result Type](#specialization-for-void-result-type)
6. [Exception Handling](#exception-handling)
7. [Usage Examples](#usage-examples)

## EnhancedPackagedTask Class Template

The `EnhancedPackagedTask` class template provides an enhanced version of `std::packaged_task` with additional features such as cancellation and completion callbacks.

### Template Parameters

- `ResultType`: The type of the result returned by the task.
- `Args...`: The types of the arguments that the task takes.

### Key Features

- Cancellable tasks
- Completion callbacks
- Integration with `EnhancedFuture`

## Constructor

```cpp
explicit EnhancedPackagedTask(TaskType task);
```

Constructs an `EnhancedPackagedTask` with the given task function.

- `task`: A callable object (function, lambda, etc.) that represents the task to be executed.

## Public Methods

### getEnhancedFuture

```cpp
EnhancedFuture<ResultType> getEnhancedFuture();
```

Returns an `EnhancedFuture` associated with the task's result.

### operator()

```cpp
void operator()(Args... args);
```

Executes the task with the provided arguments.

### onComplete

```cpp
template <typename F>
void onComplete(F&& func);
```

Sets a callback function to be called upon task completion.

### cancel

```cpp
void cancel();
```

Cancels the task.

### isCancelled

```cpp
[[nodiscard]] bool isCancelled() const;
```

Checks if the task has been cancelled.

## Protected Members

- `task_`: The callable object representing the task.
- `promise_`: The promise associated with the task's result.
- `future_`: The shared future associated with the promise.
- `callbacks_`: A vector of callback functions to be called upon task completion.
- `cancelled_`: An atomic flag indicating whether the task has been cancelled.

## Specialization for void Result Type

A specialization of `EnhancedPackagedTask` is provided for tasks that return `void`. This specialization has similar methods and behavior to the primary template, with appropriate modifications for handling tasks without a return value.

## Exception Handling

The header defines a custom exception type and macros for error handling:

- `InvalidPackagedTaskException`: A custom exception for invalid packaged task operations.
- `THROW_INVALID_PACKAGED_TASK_EXCEPTION`: Macro for throwing `InvalidPackagedTaskException`.
- `THROW_NESTED_INVALID_PACKAGED_TASK_EXCEPTION`: Macro for throwing nested `InvalidPackagedTaskException`.

## Usage Examples

Here are some examples demonstrating how to use the `EnhancedPackagedTask` class:

### Basic Usage

```cpp
#include "packaged_task.hpp"
#include <iostream>

int main() {
    // Create an EnhancedPackagedTask that adds two integers
    atom::async::EnhancedPackagedTask<int, int, int> task([](int a, int b) {
        return a + b;
    });

    // Get the associated EnhancedFuture
    auto future = task.getEnhancedFuture();

    // Execute the task
    task(5, 3);

    // Get the result
    int result = future.wait();
    std::cout << "Result: " << result << std::endl;

    return 0;
}
```

### Using Completion Callbacks

```cpp
atom::async::EnhancedPackagedTask<std::string, int> task([](int value) {
    return "Result: " + std::to_string(value);
});

task.onComplete([](const std::string& result) {
    std::cout << "Task completed. " << result << std::endl;
});

auto future = task.getEnhancedFuture();
task(42);

// The callback will be called when the task completes
future.wait();
```

### Cancelling a Task

```cpp
atom::async::EnhancedPackagedTask<void> task([]() {
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "Task completed" << std::endl;
});

auto future = task.getEnhancedFuture();

// Cancel the task before it's executed
task.cancel();

try {
    task();
} catch (const std::runtime_error& e) {
    std::cout << "Task was cancelled: " << e.what() << std::endl;
}
```

### Error Handling

```cpp
atom::async::EnhancedPackagedTask<int> task([]() {
    THROW_INVALID_PACKAGED_TASK_EXCEPTION("Something went wrong");
    return 0;
});

auto future = task.getEnhancedFuture();

try {
    task();
} catch (const atom::async::InvalidPackagedTaskException& e) {
    std::cout << "Caught exception: " << e.what() << std::endl;
}
```

These examples demonstrate the basic usage of the `EnhancedPackagedTask` class, including task execution, using completion callbacks, cancelling tasks, and error handling. Remember to handle exceptions appropriately in your actual implementations.
