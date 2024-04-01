# Async Framework

The Async Framework provides a simple way to perform asynchronous tasks in C++. It allows you to start, cancel, and retrieve the result of a task asynchronously.

## AsyncWorker Class

### Overview

Class for performing asynchronous tasks.

### Template Parameter

- `ResultType`: The type of the result returned by the task.

### Public Methods

- `void StartAsync(Func &&func, Args &&...args)`: Starts the task asynchronously.
- `ResultType GetResult()`: Gets the result of the task.
- `void Cancel()`: Cancels the task.
- `bool IsDone() const`: Checks if the task is done.
- `bool IsActive() const`: Checks if the task is active.
- `bool Validate(std::function<bool(ResultType)> validator)`: Validates the result using a validator function.
- `void SetCallback(std::function<void(ResultType)> callback)`: Sets a callback function.
- `void SetTimeout(std::chrono::seconds timeout)`: Sets a timeout for the task.
- `void WaitForCompletion()`: Waits for the task to complete.

### Private Members

- `std::future<ResultType> task_`: Represents the asynchronous task.
- `std::function<void(ResultType)> callback_`: Callback function to be called when the task is done.
- `std::chrono::seconds timeout_{0}`: Timeout duration for the task.

## AsyncWorkerManager Class

### Overview

Class for managing multiple AsyncWorker instances.

### Template Parameter

- `ResultType`: The type of the result returned by the tasks.

### Public Methods

- `std::shared_ptr<AsyncWorker<ResultType>> CreateWorker(Func &&func, Args &&...args)`: Creates a new AsyncWorker instance and starts the task asynchronously.
- `void CancelAll()`: Cancels all managed tasks.
- `bool AllDone() const`: Checks if all tasks are done.
- `void WaitForAll()`: Waits for all tasks to complete.
- `bool IsDone(std::shared_ptr<AsyncWorker<ResultType>> worker) const`: Checks if a specific task is done.
- `void Cancel(std::shared_ptr<AsyncWorker<ResultType>> worker)`: Cancels a specific task.

## Global Functions

## asyncRetry

- Async execution with retry.
- Parameters:
  - `Func func`
  - `int attemptsLeft`
  - `std::chrono::milliseconds delay`
  - `Args... args`
- Returns: A shared pointer to the created AsyncWorker instance.

## getWithTimeout

- Gets the result of the task with a timeout.
- Parameters:
  - `std::future<ReturnType> &future`
  - `std::chrono::milliseconds timeout`
- Returns: The result of the task.

## Usage Examples

```cpp
// Create an AsyncWorker object
AsyncWorker<int> worker;

// Start a task asynchronously
worker.StartAsync([](int x, int y) { return x + y; }, 5, 10);

// Wait for the task to complete
worker.WaitForCompletion();

// Get the result
int result = worker.GetResult();
```

```cpp
// Create an AsyncWorkerManager object
AsyncWorkerManager<int> manager;

// Create and start a new task
auto worker = manager.CreateWorker([]() { return 42; });

// Check if the task is done
if (manager.IsDone(worker)) {
    // Task is done
}

// Cancel the task
manager.Cancel(worker);

// Wait for all tasks to complete
manager.WaitForAll();
```

```cpp
// Using asyncRetry
auto future = asyncRetry([]() { return std::string("Hello, World!"); }, 3, std::chrono::milliseconds(100));

// Getting result with timeout
std::future<std::string> task;
std::string result = getWithTimeout(task, std::chrono::milliseconds(500));
```
