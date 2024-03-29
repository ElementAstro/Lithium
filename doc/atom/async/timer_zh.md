# 定时器

## TimerTask 类

表示由定时器安排和执行的任务。

### 构造函数

```cpp
TimerTask(std::function<void()> func, unsigned int delay, int repeatCount, int priority);
```

## 公共成员函数

- `bool operator<(const TimerTask &other) const`: 基于下一次执行时间的比较运算符。
- `void run()`: 执行任务关联的函数。
- `std::chrono::steady_clock::time_point getNextExecutionTime() const`: 获取任务的下一次计划执行时间。

### 公共成员变量

- `std::function<void()> m_func`: 要执行的函数。
- `unsigned int m_delay`: 第一次执行前的延迟。
- `int m_repeatCount`: 剩余重复次数。
- `int m_priority`: 任务的优先级。
- `std::chrono::steady_clock::time_point m_nextExecutionTime`: 下一次执行时间。

## Timer 类

表示用于安排和执行任务的定时器。

### 构造函数

```cpp
Timer();
```

### 析构函数

```cpp
~Timer();
```

### 公共成员函数

- `template <typename Function, typename... Args> std::future<typename std::result_of<Function(Args...)>::type> setTimeout(Function &&func, unsigned int delay, Args &&...args)`: 在指定延迟后安排一次性执行的任务。
- `template <typename Function, typename... Args> void setInterval(Function &&func, unsigned int interval, int repeatCount, int priority, Args &&...args)`: 安排以指定间隔重复执行的任务。
- `std::chrono::steady_clock::time_point now() const`: 获取当前时间。
- `void cancelAllTasks()`: 取消所有计划任务。
- `void pause()`: 暂停计划任务的执行。
- `void resume()`: 恢复计划任务的执行。
- `void stop()`: 停止定时器并取消所有任务。
- `template <typename Function> void setCallback(Function &&func)`: 设置在任务执行时调用的回调函数。
- `int getTaskCount() const`: 获取计划任务的数量。

### 私有成员函数

- `template <typename Function, typename... Args> std::future<typename std::result_of<Function(Args...)>::type> addTask(Function &&func, unsigned int delay, int repeatCount, int priority, Args &&...args)`: 将任务添加到任务队列中。
- `void run()`: 用于处理和运行任务的主执行循环。

### 私有成员变量

- `std::jthread m_thread` 或 `std::thread m_thread`: 用于运行定时器循环的线程。
- `std::priority_queue<TimerTask> m_taskQueue`: 用于存储计划任务的优先队列。
- `std::mutex m_mutex`: 线程同步的互斥锁。
- `std::condition_variable m_cond`: 线程同步的条件变量。
- `std::function<void()> m_callback`: 任务执行时调用的回调函数。
- `bool m_stop`: 标志，指示定时器是否应该停止。
- `bool m_paused`: 标志，指示定时器是否暂停。

### 示例用法

```cpp
// 创建一个 Timer 对象
Timer timer;

// 安排一个任务，在 1000 毫秒后执行一次
auto future = timer.setTimeout([]() {
    std::cout << "任务执行！" << std::endl;
}, 1000);

// 安排一个任务，每隔 500 毫秒重复执行 5 次
timer.setInterval([]() {
    std::cout << "重复的任务" << std::endl;
}, 500, 5, 1);

// 暂停定时器
timer.pause();

// 恢复定时器
timer.resume();

// 停止定时器
timer.stop();
```
