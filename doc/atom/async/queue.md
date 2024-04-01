# ThreadSafeQueue Template Class

The `ThreadSafeQueue` template class provides a thread-safe queue implementation in C++ for concurrent operations.

## Class Definition

```cpp
template <typename T>
struct ThreadSafeQueue : public NonCopyable {
    // Constructor
    ThreadSafeQueue();

    // Methods
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

## Usage Example

```cpp
// Example usage of ThreadSafeQueue
ThreadSafeQueue<int> tsq;

// Put elements into the queue
tsq.put(10);
tsq.put(20);
tsq.put(30);

// Take elements from the queue
std::optional<int> element = tsq.take();
if (element.has_value()) {
    std::cout << "Taken element: " << element.value() << std::endl;
}

// Check if the queue is empty
bool isEmpty = tsq.empty();
std::cout << "Is queue empty? " << (isEmpty ? "Yes" : "No") << std::endl;

// Clear the queue
tsq.clear();

// Check the size of the queue
size_t queueSize = tsq.size();
std::cout << "Queue size: " << queueSize << std::endl;
```

## Notes

- This class provides thread-safe operations for a queue data structure.
- It uses mutexes and condition variables to ensure thread safety.
- Various operations like putting, taking, sorting, transforming, and grouping elements are supported.
