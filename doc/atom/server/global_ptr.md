# GlobalSharedPtrManager Class Documentation

The `GlobalSharedPtrManager` class manages a collection of shared pointers and weak pointers. It provides functions to add, remove, and retrieve shared pointers and weak pointers by key.

## getInstance

Returns the singleton instance of the `GlobalSharedPtrManager`.

### Example

```cpp
GlobalSharedPtrManager &manager = GlobalSharedPtrManager::getInstance();
```

## getSharedPtr

Retrieves a shared pointer from the shared pointer map with the specified key.

### Example

```cpp
auto sharedPtr = manager.getSharedPtr<int>("key1");
if (sharedPtr) {
    // Shared pointer found
} else {
    // Shared pointer not found
}
```

## getWeakPtr

Retrieves a weak pointer from the shared pointer map with the specified key.

### Example

```cpp
auto weakPtr = manager.getWeakPtr<int>("key2");
if (!weakPtr.expired()) {
    auto sharedPtr = weakPtr.lock();
    // Use sharedPtr
} else {
    // Weak pointer expired
}
```

## addSharedPtr

Adds a shared pointer to the shared pointer map with the specified key.

### Example

```cpp
std::shared_ptr<float> mySharedPtr = std::make_shared<float>(3.14);
manager.addSharedPtr("key3", mySharedPtr);
```

## removeSharedPtr

Removes a shared pointer from the shared pointer map with the specified key.

### Example

```cpp
manager.removeSharedPtr("key3");
```

## addWeakPtr

Adds a weak pointer to the shared pointer map with the specified key.

### Example

```cpp
std::weak_ptr<std::string> myWeakPtr = mySharedPtr;
manager.addWeakPtr("key4", myWeakPtr);
```

## getSharedPtrFromWeakPtr

Retrieves a shared pointer from a weak pointer in the shared pointer map with the specified key.

### Example

```cpp
auto sharedPtr = manager.getSharedPtrFromWeakPtr<std::string>("key4");
if (sharedPtr) {
    // Shared pointer retrieved successfully
} else {
    // Weak pointer expired or shared object deleted
}
```

## removeExpiredWeakPtrs

Removes all expired weak pointers from the shared pointer map.

### Example

```cpp
manager.removeExpiredWeakPtrs();
```

## addDeleter

Adds a custom deleter function for the shared object associated with the specified key.

### Example

```cpp
auto customDeleter = [](int *ptr) { delete ptr; };
manager.addDeleter("key5", customDeleter);
```

## deleteObject

Deletes the shared object associated with the specified key.

### Example

```cpp
int *ptr = new int(42);
manager.deleteObject("key5", ptr);
```

## printSharedPtrMap

Prints the contents of the shared pointer map.

### Example

```cpp
manager.printSharedPtrMap();
```
