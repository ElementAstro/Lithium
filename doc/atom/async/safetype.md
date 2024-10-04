# C++ Concurrent Data Structures Documentation

This document provides a detailed explanation of the concurrent data structures and their usage as defined in the `atom::async` namespace.

## Table of Contents

1. [LockFreeStack](#lockfreestack)
2. [LockFreeHashTable](#lockfreehashtable)
3. [ThreadSafeVector](#threadsafevector)
4. [LockFreeList](#lockfreelist)

## LockFreeStack

### Overview

`LockFreeStack` is a lock-free implementation of a stack data structure, suitable for concurrent use.

### Template Parameters

- `T`: Type of elements stored in the stack.

### Public Methods

#### Constructor

```cpp
LockFreeStack();
```

Creates an empty lock-free stack.

#### Destructor

```cpp
~LockFreeStack();
```

Destroys the stack and frees all allocated memory.

#### push

```cpp
void push(const T& value);
void push(T&& value);
```

Pushes a value onto the stack. Supports both lvalue and rvalue references.

#### pop

```cpp
auto pop() -> std::optional<T>;
```

Attempts to pop the top value off the stack. Returns an `std::optional<T>` containing the value if successful, or `std::nullopt` if the stack is empty.

#### top

```cpp
auto top() const -> std::optional<T>;
```

Returns the top value of the stack without removing it. Returns `std::nullopt` if the stack is empty.

#### empty

```cpp
[[nodiscard]] auto empty() const -> bool;
```

Checks if the stack is empty.

#### size

```cpp
[[nodiscard]] auto size() const -> int;
```

Returns the approximate number of elements in the stack.

### Usage Example

```cpp
atom::async::LockFreeStack<int> stack;

// Pushing elements
stack.push(1);
stack.push(2);
stack.push(3);

// Popping elements
auto top = stack.pop(); // Returns 3
auto second = stack.pop(); // Returns 2

// Checking if empty
bool isEmpty = stack.empty(); // Returns false

// Getting size
int size = stack.size(); // Returns 1
```

## LockFreeHashTable

### Overview

`LockFreeHashTable` is a lock-free implementation of a hash table, allowing concurrent access and modification.

### Template Parameters

- `Key`: Type of the keys in the hash table.
- `Value`: Type of the values associated with the keys.

### Public Methods

#### Constructor

```cpp
explicit LockFreeHashTable(size_t num_buckets = 16);
```

Creates a lock-free hash table with the specified number of buckets (default is 16).

#### find

```cpp
auto find(const Key& key) const -> std::optional<Value>;
```

Searches for a key in the hash table. Returns an `std::optional<Value>` containing the associated value if found, or `std::nullopt` if not found.

#### insert

```cpp
void insert(const Key& key, const Value& value);
```

Inserts a key-value pair into the hash table.

#### erase

```cpp
void erase(const Key& key);
```

Removes the entry with the specified key from the hash table.

#### empty

```cpp
[[nodiscard]] auto empty() const -> bool;
```

Checks if the hash table is empty.

#### size

```cpp
[[nodiscard]] auto size() const -> size_t;
```

Returns the number of elements in the hash table.

#### clear

```cpp
void clear();
```

Removes all elements from the hash table.

### Iterator

The `LockFreeHashTable` provides a forward iterator for traversing the elements.

```cpp
auto begin() -> Iterator;
auto end() -> Iterator;
```

### Usage Example

```cpp
atom::async::LockFreeHashTable<std::string, int> hashTable;

// Inserting elements
hashTable.insert("one", 1);
hashTable.insert("two", 2);

// Finding elements
auto value = hashTable.find("one"); // Returns std::optional containing 1
auto notFound = hashTable.find("three"); // Returns std::nullopt

// Erasing elements
hashTable.erase("two");

// Checking if empty
bool isEmpty = hashTable.empty(); // Returns false

// Getting size
size_t size = hashTable.size(); // Returns 1

// Iterating over elements
for (const auto& [key, value] : hashTable) {
    std::cout << key << ": " << value << std::endl;
}
```

## ThreadSafeVector

### Overview

`ThreadSafeVector` is a thread-safe implementation of a dynamic array, allowing concurrent access and modification.

### Template Parameters

- `T`: Type of elements stored in the vector.

### Public Methods

#### Constructor

```cpp
explicit ThreadSafeVector(size_t initial_capacity = 16);
```

Creates a thread-safe vector with the specified initial capacity (default is 16).

#### Destructor

```cpp
~ThreadSafeVector();
```

Destroys the vector and frees all allocated memory.

#### pushBack

```cpp
void pushBack(const T& value);
void pushBack(T&& value);
```

Adds an element to the end of the vector. Supports both lvalue and rvalue references.

#### popBack

```cpp
auto popBack() -> std::optional<T>;
```

Removes and returns the last element of the vector. Returns `std::nullopt` if the vector is empty.

#### at

```cpp
auto at(size_t index) const -> std::optional<T>;
```

Returns the element at the specified index. Returns `std::nullopt` if the index is out of range.

#### empty

```cpp
auto empty() const -> bool;
```

Checks if the vector is empty.

#### getSize

```cpp
auto getSize() const -> size_t;
```

Returns the number of elements in the vector.

#### getCapacity

```cpp
auto getCapacity() const -> size_t;
```

Returns the current capacity of the vector.

#### clear

```cpp
void clear();
```

Removes all elements from the vector.

#### shrinkToFit

```cpp
void shrinkToFit();
```

Reduces the capacity of the vector to fit its size.

#### front

```cpp
auto front() const -> T;
```

Returns the first element of the vector. Throws an exception if the vector is empty.

#### back

```cpp
auto back() const -> T;
```

Returns the last element of the vector. Throws an exception if the vector is empty.

#### operator[]

```cpp
auto operator[](size_t index) const -> T;
```

Returns the element at the specified index. Throws an exception if the index is out of range.

### Usage Example

```cpp
atom::async::ThreadSafeVector<int> vector;

// Adding elements
vector.pushBack(1);
vector.pushBack(2);
vector.pushBack(3);

// Accessing elements
auto firstElement = vector.front(); // Returns 1
auto lastElement = vector.back(); // Returns 3
auto elementAtIndex = vector[1]; // Returns 2

// Removing elements
auto poppedElement = vector.popBack(); // Returns 3

// Checking size and capacity
auto size = vector.getSize(); // Returns 2
auto capacity = vector.getCapacity(); // Returns a value >= 2

// Clearing the vector
vector.clear();

// Checking if empty
bool isEmpty = vector.empty(); // Returns true
```

## LockFreeList

### Overview

`LockFreeList` is a lock-free implementation of a singly-linked list, allowing concurrent access and modification.

### Template Parameters

- `T`: Type of elements stored in the list.

### Public Methods

#### Constructor

```cpp
LockFreeList();
```

Creates an empty lock-free list.

#### Destructor

```cpp
~LockFreeList();
```

Destroys the list and frees all allocated memory.

#### pushFront

```cpp
void pushFront(T value);
```

Adds an element to the front of the list.

#### popFront

```cpp
auto popFront() -> std::optional<T>;
```

Removes and returns the first element of the list. Returns `std::nullopt` if the list is empty.

#### empty

```cpp
[[nodiscard]] auto empty() const -> bool;
```

Checks if the list is empty.

### Iterator

The `LockFreeList` provides a forward iterator for traversing the elements.

```cpp
auto begin() -> Iterator;
auto end() -> Iterator;
```

### Usage Example

```cpp
atom::async::LockFreeList<int> list;

// Adding elements
list.pushFront(3);
list.pushFront(2);
list.pushFront(1);

// Removing elements
auto firstElement = list.popFront(); // Returns 1
auto secondElement = list.popFront(); // Returns 2

// Checking if empty
bool isEmpty = list.empty(); // Returns false

// Iterating over elements
for (const auto& value : list) {
    std::cout << value << std::endl; // Prints: 3
}
```

## General Notes on Usage

1. **Thread Safety**: All these data structures are designed for concurrent use. They can be safely accessed and modified from multiple threads without external synchronization.

2. **Memory Management**: These structures use various techniques for safe memory management in a concurrent environment, such as hazard pointers in the `LockFreeList`.

3. **Performance Considerations**: While these structures provide thread-safety, they may have different performance characteristics compared to their non-concurrent counterparts. The choice between these and other synchronization methods (like mutex-protected standard containers) depends on the specific use case and performance requirements.

4. **Exception Safety**: These structures are designed to be exception-safe. However, they assume that the operations on the contained objects (like copy constructors, move constructors, and destructors) do not throw exceptions.

5. **Iteration**: The `LockFreeHashTable` and `LockFreeList` provide iterators. However, be aware that iterating over these structures while other threads are modifying them may not provide a consistent view of the data.

6. **Optional Return Values**: Many methods return `std::optional<T>` instead of throwing exceptions or returning sentinel values. This provides a clear and type-safe way to handle cases where an operation cannot be completed (e.g., popping from an empty container).

7. **Move Semantics**: Where applicable, these structures support move semantics for efficient insertion and removal of elements.

## Best Practices

1. **Choosing the Right Structure**: Select the appropriate data structure based on your specific needs. For example, use `LockFreeStack` for LIFO operations, `LockFreeHashTable` for key-value pairs with fast lookup, `ThreadSafeVector` for dynamic arrays, and `LockFreeList` for scenarios requiring frequent insertion at the beginning.

2. **Error Handling**: Always check the return values of methods that return `std::optional<T>` before using the returned value.

3. **Iterators**: When using iterators, be aware that the structure may change due to concurrent modifications. If you need a consistent view, consider using other synchronization mechanisms.

4. **Memory Usage**: Be mindful of the memory usage, especially with `ThreadSafeVector`. Use `shrinkToFit()` when appropriate to optimize memory consumption.

5. **Performance Tuning**: For `LockFreeHashTable`, choose an appropriate initial number of buckets based on the expected number of elements to minimize collisions and resizing operations.

By following these guidelines and understanding the characteristics of each data structure, you can effectively use these concurrent data structures in your multithreaded C++ applications.
