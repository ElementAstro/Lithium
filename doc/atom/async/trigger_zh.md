# 触发器类

`Trigger` 类提供了一个机制，用于注册特定事件的回调函数，并在事件发生时触发这些回调。它支持使用不同优先级注册回调、使用延迟调度触发器以及异步触发事件。

## 使用示例

```cpp
// 定义一个带有 int 参数类型的 Trigger 对象
Trigger<int> trigger;

// 定义一个回调函数
Trigger<int>::Callback callback = [](int param) {
    std::cout << "触发回调，参数为：" << param << std::endl;
};

// 为事件注册回调
trigger.registerCallback("event1", callback, Trigger<int>::CallbackPriority::Normal);

// 触发带有参数 42 的事件
trigger.trigger("event1", 42);
```

## 方法

### `registerCallback`

为特定事件注册回调函数。

### `unregisterCallback`

取消特定事件的回调函数注册。

### `trigger`

触发为特定事件注册的回调函数。

### `scheduleTrigger`

使用指定延迟调度事件触发。

### `scheduleAsyncTrigger`

调度异步触发事件并返回一个 future 对象。

### `cancelTrigger`

取消特定事件的调度触发。

### `cancelAllTriggers`

取消所有事件的调度触发。

## 私有成员

- `m_mutex`：用于同步访问回调数据结构的互斥锁。
- `m_callbacks`：哈希映射，用于存储事件的注册回调函数。

### 调度触发器的示例用法

```cpp
// 使用 500 毫秒的延迟调度一个事件触发
trigger.scheduleTrigger("event2", 55, std::chrono::milliseconds(500));
```

### 异步触发的示例用法

```cpp
// 调度一个异步触发，并获取一个 future 对象
std::future<void> future = trigger.scheduleAsyncTrigger("event3", 77);

// 等待异步触发完成
future.wait();
```
