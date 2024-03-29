# 异步框架

异步框架提供了在 C++中执行异步任务的简单方式。它允许您异步启动、取消和检索任务的结果。

## AsyncWorker 类

### 概述

用于执行异步任务的类。

### 模板参数

- `ResultType`: 任务返回的结果类型。

### 公共方法

- `void StartAsync(Func &&func, Args &&...args)`: 异步启动任务。
- `ResultType GetResult()`: 获取任务的结果。
- `void Cancel()`: 取消任务。
- `bool IsDone() const`: 检查任务是否已完成。
- `bool IsActive() const`: 检查任务是否处于活动状态。
- `bool Validate(std::function<bool(ResultType)> validator)`: 使用验证函数验证结果。
- `void SetCallback(std::function<void(ResultType)> callback)`: 设置回调函数。
- `void SetTimeout(std::chrono::seconds timeout)`: 设置任务的超时时间。
- `void WaitForCompletion()`: 等待任务完成。

### 私有成员

- `std::future<ResultType> task_`: 表示异步任务。
- `std::function<void(ResultType)> callback_`: 任务完成时要调用的回调函数。
- `std::chrono::seconds timeout_{0}`: 任务的超时持续时间。

## AsyncWorkerManager 类

### 概述

用于管理多个 AsyncWorker 实例的类。

### 模板参数

- `ResultType`: 任务返回的结果类型。

### 公共方法

- `std::shared_ptr<AsyncWorker<ResultType>> CreateWorker(Func &&func, Args &&...args)`: 创建新的 AsyncWorker 实例并异步启动任务。
- `void CancelAll()`: 取消所有管理的任务。
- `bool AllDone() const`: 检查所有任务是否已完成。
- `void WaitForAll()`: 等待所有任务完成。
- `bool IsDone(std::shared_ptr<AsyncWorker<ResultType>> worker) const`: 检查特定任务是否已完成。
- `void Cancel(std::shared_ptr<AsyncWorker<ResultType>> worker)`: 取消特定任务。

## 全局函数

## asyncRetry

- 带重试的异步执行。
- 参数:
  - `Func func`
  - `int attemptsLeft`
  - `std::chrono::milliseconds delay`
  - `Args... args`
- 返回值: 创建的 AsyncWorker 实例的共享指针。

## getWithTimeout

- 使用超时获取任务的结果。
- 参数:
  - `std::future<ReturnType> &future`
  - `std::chrono::milliseconds timeout`
- 返回值: 任务的结果。

## 用法示例

```cpp
// 创建一个AsyncWorker对象
AsyncWorker<int> worker;

// 异步启动任务
worker.StartAsync([](int x, int y) { return x + y; }, 5, 10);

// 等待任务完成
worker.WaitForCompletion();

// 获取结果
int result = worker.GetResult();
```

```cpp
// 创建一个AsyncWorkerManager对象
AsyncWorkerManager<int> manager;

// 创建并启动新任务
auto worker = manager.CreateWorker([]() { return 42; });

// 检查任务是否已完成
if (manager.IsDone(worker)) {
    // 任务已完成
}

// 取消任务
manager.Cancel(worker);

// 等待所有任务完成
manager.WaitForAll();
```

```cpp
// 使用asyncRetry
auto future = asyncRetry([]() { return std::string("Hello, World!"); }, 3, std::chrono::milliseconds(100));

// 使用超时获取结果
std::future<std::string> task;
std::string result = getWithTimeout(task, std::chrono::milliseconds(500));
```
