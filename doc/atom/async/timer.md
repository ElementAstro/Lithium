# Timer Class Documentation

## Overview

The `Timer` class is designed for scheduling and executing tasks at specified intervals or after a delay. It is defined in the `atom::async` namespace and is part of the `timer.hpp` header file. This class provides functionality for setting timeouts, intervals, and managing task execution with priorities.

## Class Declarations

### TimerTask

```cpp
class TimerTask {
public:
    explicit TimerTask(std::function<void()> func, unsigned int delay,
                       int repeatCount, int priority);
    auto operator<(const TimerTask &other) const -> bool;
    void run();
    auto getNextExecutionTime() const -> std::chrono::steady_clock::time_point;

    // Member variables
    std::function<void()> m_func;
    unsigned int m_delay;
    int m_repeatCount;
    int m_priority;
    std::chrono::steady_clock::time_point m_nextExecutionTime;
};
```

### Timer

```cpp
class Timer {
public:
    Timer();
    ~Timer();

    // Public member functions
    // ... (detailed below)

private:
    // Private member functions and variables
    // ... (detailed below)
};
```

## Timer Class Member Functions

### Public Member Functions

#### setTimeout

```cpp
template <typename Function, typename... Args>
[[nodiscard]] auto setTimeout(Function &&func, unsigned int delay,
                              Args &&...args)
    -> std::future<typename std::result_of<Function(Args...)>::type>;
```

Schedules a task to be executed once after a specified delay.

- **Parameters:**
  - `func`: The function to be executed.
  - `delay`: The delay in milliseconds before the function is executed.
  - `args`: The arguments to be passed to the function.
- **Returns:** A future representing the result of the function execution.

#### setInterval

```cpp
template <typename Function, typename... Args>
void setInterval(Function &&func, unsigned int interval, int repeatCount,
                 int priority, Args &&...args);
```

Schedules a task to be executed repeatedly at a specified interval.

- **Parameters:**
  - `func`: The function to be executed.
  - `interval`: The interval in milliseconds between executions.
  - `repeatCount`: The number of times the function should be repeated. -1 for infinite repetition.
  - `priority`: The priority of the task.
  - `args`: The arguments to be passed to the function.

#### now

```cpp
[[nodiscard]] auto now() const -> std::chrono::steady_clock::time_point;
```

Gets the current time point.

- **Returns:** The current time point of the steady clock.

#### cancelAllTasks

```cpp
void cancelAllTasks();
```

Cancels all scheduled tasks.

#### pause

```cpp
void pause();
```

Pauses the execution of scheduled tasks.

#### resume

```cpp
void resume();
```

Resumes the execution of scheduled tasks after pausing.

#### stop

```cpp
void stop();
```

Stops the timer and cancels all tasks.

#### setCallback

```cpp
template <typename Function>
void setCallback(Function &&func);
```

Sets a callback function to be called when a task is executed.

- **Parameters:**
  - `func`: The callback function to be set.

#### getTaskCount

```cpp
[[nodiscard]] auto getTaskCount() const -> size_t;
```

Gets the number of tasks currently in the queue.

- **Returns:** The number of tasks in the queue.

### Private Member Functions

#### addTask

```cpp
template <typename Function, typename... Args>
auto addTask(Function &&func, unsigned int delay, int repeatCount,
             int priority, Args &&...args)
    -> std::future<typename std::result_of<Function(Args...)>::type>;
```

Adds a task to the task queue.

- **Parameters:**
  - `func`: The function to be executed.
  - `delay`: The delay in milliseconds before the function is executed.
  - `repeatCount`: The number of repetitions remaining.
  - `priority`: The priority of the task.
  - `args`: The arguments to be passed to the function.
- **Returns:** A future representing the result of the function execution.

#### run

```cpp
void run();
```

Main execution loop for processing and running tasks.

## Usage Example

Here's a simple example demonstrating how to use the `Timer` class:

```cpp
#include "timer.hpp"
#include <iostream>
#include <chrono>

int main() {
    atom::async::Timer timer;

    // Set a timeout
    timer.setTimeout([]() {
        std::cout << "This will be executed after 2 seconds." << std::endl;
    }, 2000);

    // Set an interval
    timer.setInterval([]() {
        std::cout << "This will be executed every 1 second, 5 times." << std::endl;
    }, 1000, 5, 0);

    // Set a callback
    timer.setCallback([]() {
        std::cout << "A task has been executed." << std::endl;
    });

    // Wait for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // Stop the timer
    timer.stop();

    return 0;
}
```

This example creates a `Timer` object, sets a timeout for a one-time task, sets an interval for a repeating task, and sets a callback to be called after each task execution. It then waits for 10 seconds before stopping the timer.

## Notes

- The `Timer` class uses a priority queue to manage tasks, allowing for efficient scheduling based on execution time and priority.
- Tasks can be scheduled for one-time execution (setTimeout) or repeated execution (setInterval).
- The class is thread-safe, using mutexes and condition variables for synchronization.
- The timer can be paused, resumed, and stopped, providing flexible control over task execution.
- A callback function can be set to be called after each task execution, useful for logging or monitoring.
- The class uses C++11 features and can utilize `std::jthread` if C++20 or later is available.
