# 线程安全队列模板类

`ThreadSafeQueue` 模板类提供了一个用于并发操作的线程安全队列实现，适用于 C++ 语言。

## 类定义

```cpp
template <typename T>
struct ThreadSafeQueue : public NonCopyable {
    // 构造函数
    ThreadSafeQueue();

    // 方法
    void put(T element);
    std::optional<T> take();
    std::queue<T> destroy();
    size_t size() const;
    bool empty() const;
    void clear();
    std::optional<T> front();
    std::optional<T> back();
    template <typename... Args>
    void emplace(Args &&...args);
    template <typename Predicate>
    std::optional<T> waitFor(Predicate predicate);
    void waitUntilEmpty();
    template <typename UnaryPredicate>
    std::vector<T> extractIf(UnaryPredicate pred);
    template <typename Compare>
    void sort(Compare comp);
    template <typename ResultType>
    ThreadSafeQueue<ResultType> transform(std::function<ResultType(T)> func);
    template <typename GroupKey>
    std::vector<ThreadSafeQueue<T>> groupBy(std::function<GroupKey(T)> func);
};
```

## 用法示例

```cpp
// ThreadSafeQueue 的示例用法
ThreadSafeQueue<int> tsq;

// 将元素放入队列
tsq.put(10);
tsq.put(20);
tsq.put(30);

// 从队列中取出元素
std::optional<int> element = tsq.take();
if (element.has_value()) {
    std::cout << "取出的元素: " << element.value() << std::endl;
}

// 检查队列是否为空
bool isEmpty = tsq.empty();
std::cout << "队列是否为空？" << (isEmpty ? "是" : "否") << std::endl;

// 清空队列
tsq.clear();

// 检查队列大小
size_t queueSize = tsq.size();
std::cout << "队列大小: " << queueSize << std::endl;
```

## 注意事项

- 该类为队列数据结构提供了线程安全的操作。
- 使用互斥锁和条件变量确保线程安全。
- 支持各种操作，如放置、取出、排序、转换和分组元素。
