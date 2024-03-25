# Timer Module Documentation

## TimerTask Class

Represents a task to be scheduled and executed by the Timer.

### Constructor

```cpp
TimerTask(std::function<void()> func, unsigned int delay, int repeatCount, int priority);
```

## Public Member Functions

- `bool operator<(const TimerTask &other) const`: Comparison operator based on next execution time.
- `void run()`: Executes the task's associated function.
- `std::chrono::steady_clock::time_point getNextExecutionTime() const`: Get the next scheduled execution time of the task.

### Public Members

- `std::function<void()> m_func`: The function to be executed.
- `unsigned int m_delay`: The delay before the first execution.
- `int m_repeatCount`: The number of repetitions remaining.
- `int m_priority`: The priority of the task.
- `std::chrono::steady_clock::time_point m_nextExecutionTime`: The next execution time.

## Timer Class

Represents a timer for scheduling and executing tasks.

### Constructor

```cpp
Timer();
```

### Destructor

```cpp
~Timer();
```

### Public Member Functions

- `template <typename Function, typename... Args> std::future<typename std::result_of<Function(Args...)>::type> setTimeout(Function &&func, unsigned int delay, Args &&...args)`: Schedules a task to be executed once after a specified delay.
- `template <typename Function, typename... Args> void setInterval(Function &&func, unsigned int interval, int repeatCount, int priority, Args &&...args)`: Schedules a task to be executed repeatedly at a specified interval.
- `std::chrono::steady_clock::time_point now() const`: Get the current time.
- `void cancelAllTasks()`: Cancels all scheduled tasks.
- `void pause()`: Pauses the execution of scheduled tasks.
- `void resume()`: Resumes the execution of scheduled tasks.
- `void stop()`: Stops the timer and cancels all tasks.
- `template <typename Function> void setCallback(Function &&func)`: Sets a callback function to be called when a task is executed.
- `int getTaskCount() const`: Get the count of scheduled tasks.

### Private Member Functions

- `template <typename Function, typename... Args> std::future<typename std::result_of<Function(Args...)>::type> addTask(Function &&func, unsigned int delay, int repeatCount, int priority, Args &&...args)`: Adds a task to the task queue.
- `void run()`: Main execution loop for processing and running tasks.

### Private Members

- `std::jthread m_thread` or `std::thread m_thread`: The thread for running the timer loop.
- `std::priority_queue<TimerTask> m_taskQueue`: The priority queue for scheduled tasks.
- `std::mutex m_mutex`: Mutex for thread synchronization.
- `std::condition_variable m_cond`: Condition variable for thread synchronization.
- `std::function<void()> m_callback`: The callback function to be called when a task is executed.
- `bool m_stop`: Flag indicating whether the timer should stop.
- `bool m_paused`: Flag indicating whether the timer is paused.

### Example Usage

```cpp
// Create a Timer object
Timer timer;

// Schedule a task to be executed once after 1000 milliseconds
auto future = timer.setTimeout([]() {
    std::cout << "Task executed!" << std::endl;
}, 1000);

// Schedule a task to be repeated every 500 milliseconds, 5 times
timer.setInterval([]() {
    std::cout << "Repeated task" << std::endl;
}, 500, 5, 1);

// Pause the timer
timer.pause();

// Resume the timer
timer.resume();

// Stop the timer
timer.stop();
```
