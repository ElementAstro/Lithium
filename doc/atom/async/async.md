# Documentation for async.hpp

This document provides a detailed overview of the `async.hpp` header file, which contains classes and functions for managing asynchronous tasks in C++.

## Table of Contents

1. [AsyncWorker Class](#asyncworker-class)
2. [AsyncWorkerManager Class](#asyncworkermanager-class)
3. [Utility Functions](#utility-functions)
4. [Enums and Types](#enums-and-types)

## AsyncWorker Class

The `AsyncWorker` class is a template class for performing asynchronous tasks. It allows you to start a task asynchronously and retrieve the result when it's done.

### Template Parameters

- `ResultType`: The type of the result returned by the task.

### Methods

#### startAsync

```cpp
template <typename Func, typename... Args>
void startAsync(Func&& func, Args&&... args);
```

Starts the task asynchronously.

- `func`: The function to be executed asynchronously.
- `args`: The arguments to be passed to the function.

#### getResult

```cpp
auto getResult() -> ResultType;
```

Gets the result of the task. Throws a `std::runtime_error` if the task is not valid.

#### cancel

```cpp
void cancel();
```

Cancels the task. If the task is valid, this function waits for the task to complete.

#### isDone

```cpp
[[nodiscard]] auto isDone() const -> bool;
```

Checks if the task is done.

#### isActive

```cpp
[[nodiscard]] auto isActive() const -> bool;
```

Checks if the task is active.

#### validate

```cpp
auto validate(std::function<bool(ResultType)> validator) -> bool;
```

Validates the result of the task using a validator function.

#### setCallback

```cpp
void setCallback(std::function<void(ResultType)> callback);
```

Sets a callback function to be called when the task is done.

#### setTimeout

```cpp
void setTimeout(std::chrono::seconds timeout);
```

Sets a timeout for the task.

#### waitForCompletion

```cpp
void waitForCompletion();
```

Waits for the task to complete. If a timeout is set, it waits until the task is done or the timeout is reached.

## AsyncWorkerManager Class

The `AsyncWorkerManager` class manages multiple `AsyncWorker` instances.

### Template Parameters

- `ResultType`: The type of the result returned by the tasks managed by this class.

### Methods

#### createWorker

```cpp
template <typename Func, typename... Args>
auto createWorker(Func&& func, Args&&... args) -> std::shared_ptr<AsyncWorker<ResultType>>;
```

Creates a new `AsyncWorker` instance and starts the task asynchronously.

#### cancelAll

```cpp
void cancelAll();
```

Cancels all the managed tasks.

#### allDone

```cpp
auto allDone() const -> bool;
```

Checks if all the managed tasks are done.

#### waitForAll

```cpp
void waitForAll();
```

Waits for all the managed tasks to complete.

#### isDone

```cpp
bool isDone(std::shared_ptr<AsyncWorker<ResultType>> worker) const;
```

Checks if a specific task is done.

#### cancel

```cpp
void cancel(std::shared_ptr<AsyncWorker<ResultType>> worker);
```

Cancels a specific task.

## Utility Functions

### getWithTimeout

```cpp
template <typename ReturnType>
auto getWithTimeout(std::future<ReturnType>& future, std::chrono::milliseconds timeout) -> ReturnType;
```

Gets the result of the task with a timeout.

### asyncRetry

```cpp
template <typename Func, typename Callback, typename ExceptionHandler, typename CompleteHandler, typename... Args>
auto asyncRetry(Func&& func, int attemptsLeft, std::chrono::milliseconds initialDelay,
                BackoffStrategy strategy, std::chrono::milliseconds maxTotalDelay,
                Callback&& callback, ExceptionHandler&& exceptionHandler,
                CompleteHandler&& completeHandler, Args&&... args)
    -> std::future<typename std::invoke_result_t<Func, Args...>>;
```

Executes an asynchronous function with retry capabilities.

## Enums and Types

### BackoffStrategy

An enum class for different backoff strategies:

```cpp
enum class BackoffStrategy { FIXED, LINEAR, EXPONENTIAL };
```

### EnableIfNotVoid

A type alias for enabling functions only if the template parameter is not void:

```cpp
template <typename T>
using EnableIfNotVoid = typename std::enable_if_t<!std::is_void_v<T>, T>;
```

## Usage Examples

Here are some basic usage examples for the main components:

### Using AsyncWorker

```cpp
AsyncWorker<int> worker;
worker.startAsync([]() {
    // Some long-running task
    return 42;
});
worker.setCallback([](int result) {
    std::cout << "Task completed with result: " << result << std::endl;
});
worker.waitForCompletion();
```

### Using AsyncWorkerManager

```cpp
AsyncWorkerManager<int> manager;
auto worker1 = manager.createWorker([]() { return 1; });
auto worker2 = manager.createWorker([]() { return 2; });
manager.waitForAll();
```

### Using asyncRetry

```cpp
auto result = asyncRetry(
    []() { /* Your async function */ },
    5, // Number of attempts
    std::chrono::milliseconds(100), // Initial delay
    BackoffStrategy::EXPONENTIAL,
    std::chrono::milliseconds(5000), // Max total delay
    []() { std::cout << "Attempt succeeded" << std::endl; },
    [](const std::exception& e) { std::cout << "Exception: " << e.what() << std::endl; },
    []() { std::cout << "All attempts completed" << std::endl; }
);
```
