# ResourceCache Class Documentation

The ResourceCache class provides a cache for storing and managing resources of type T. It supports functionalities to insert, retrieve, remove, and manage resources with expiration times. This document provides an overview of the class and usage examples for each member function.

## Class Template

```cpp
template <typename T>
class ResourceCache {
    // ... (class definition as provided)
};
```

## Constructor

### ResourceCache(int maxSize)

Constructs a ResourceCache with a maximum size.

#### Usage Example

```cpp
// Create a cache with a maximum size of 100
ResourceCache<int> cache(100);
```

---

## Member Functions

### insert

Inserts a resource into the cache with an expiration time.

```cpp
void insert(const std::string &key, const T &value, std::chrono::seconds expirationTime);
```

#### Usage Example

```cpp
// Insert a resource with key "data1", value 42, and expiration time of 60 seconds
cache.insert("data1", 42, std::chrono::seconds(60));
```

### contains

Checks if the cache contains a resource with the given key.

```cpp
bool contains(const std::string &key) const;
```

#### Usage Example

```cpp
// Check if the cache contains the resource with key "data1"
if (cache.contains("data1")) {
    std::cout << "Resource found in the cache." << std::endl;
} else {
    std::cout << "Resource not found in the cache." << std::endl;
}
```

### get

Retrieves a resource from the cache by key.

```cpp
const T &get(const std::string &key);
```

#### Usage Example

```cpp
// Retrieve the resource with key "data1" from the cache
int data = cache.get("data1");
std::cout << "Retrieved data: " << data << std::endl;
```

### remove

Removes a resource from the cache by key.

```cpp
void remove(const std::string &key);
```

#### Usage Example

```cpp
// Remove the resource with key "data1" from the cache
cache.remove("data1");
```

### asyncGet

Retrieves a resource from the cache by key asynchronously.

```cpp
std::future<T> asyncGet(const std::string &key);
```

#### Usage Example

```cpp
// Retrieve the resource with key "data1" from the cache asynchronously
std::future<int> futureData = cache.asyncGet("data1");
// Wait for the future and get the retrieved data
int data = futureData.get();
std::cout << "Retrieved data: " << data << std::endl;
```

### asyncInsert

Inserts a resource into the cache with an expiration time asynchronously.

```cpp
std::future<void> asyncInsert(const std::string &key, const T &value, const std::chrono::seconds &expirationTime);
```

#### Usage Example

```cpp
// Insert a resource with key "data1", value 42, and expiration time of 60 seconds asynchronously
auto futureInsert = cache.asyncInsert("data1", 42, std::chrono::seconds(60));
// Wait for the insertion to complete
futureInsert.wait();
```

### clear

Clears the cache.

```cpp
void clear();
```

#### Usage Example

```cpp
// Clear the cache
cache.clear();
```

### size

Returns the number of elements in the cache.

```cpp
size_t size() const;
```

#### Usage Example

```cpp
// Get the number of elements in the cache
size_t numElements = cache.size();
std::cout << "Number of elements in the cache: " << numElements << std::endl;
```

### empty

Checks if the cache is empty.

```cpp
bool empty() const;
```

#### Usage Example

```cpp
// Check if the cache is empty
if (cache.empty()) {
    std::cout << "The cache is empty." << std::endl;
} else {
    std::cout << "The cache is not empty." << std::endl;
}
```

### evictOldest

Evicts the oldest resource from the cache.

```cpp
void evictOldest();
```

#### Usage Example

```cpp
// Evict the oldest resource from the cache
cache.evictOldest();
```

### isExpired

Checks if a resource with the given key has expired.

```cpp
bool isExpired(const std::string &key) const;
```

#### Usage Example

```cpp
// Check if the resource with key "data1" has expired
if (cache.isExpired("data1")) {
    std::cout << "The resource has expired." << std::endl;
} else {
    std::cout << "The resource has not expired." << std::endl;
}
```

### asyncLoad

Loads a resource asynchronously.

```cpp
std::future<void> asyncLoad(const std::string &key, std::function<T()> loadDataFunction);
```

#### Usage Example

```cpp
// Load the resource with key "data1" asynchronously using a custom load function
std::future<void> futureLoad = cache.asyncLoad("data1", []() {
    // Custom load function implementation
    return 42;
});
// Wait for the loading to complete
futureLoad.wait();
```

### setMaxSize

Sets the maximum size of the cache.

```cpp
void setMaxSize(int maxSize);
```

#### Usage Example

```cpp
// Set the maximum size of the cache to 200
cache.setMaxSize(200);
```

### setExpirationTime

Sets the expiration time of a resource.

```cpp
void setExpirationTime(const std::string &key, std::chrono::seconds expirationTime);
```

#### Usage Example

```cpp
// Set the expiration time of the resource with key "data1" to 120 seconds
cache.setExpirationTime("data1", std::chrono::seconds(120));
```

### readFromFile

Reads a resource from a file asynchronously.

```cpp
void readFromFile(const std::string &filePath, const std::function<T(const std::string &)> &deserializer);
```

#### Usage Example

```cpp
// Read a resource from a file asynchronously and deserialize using a custom deserializer function
cache.readFromFile("data.txt", [](const std::string &data) {
    // Custom deserialization logic
    return stoi(data);
});
```

### writeToFile

Writes a resource to a file asynchronously.

```cpp
void writeToFile(const std::string &filePath, const std::function<std::string(const T &)> &serializer);
```

#### Usage Example

```cpp
// Write a resource to a file asynchronously using a custom serializer function
cache.writeToFile("data.txt", [](const int &data) {
    // Custom serialization logic
    return std::to_string(data);
});
```

### removeExpired

Removes expired resources from the cache.

```cpp
void removeExpired();
```

#### Usage Example

```cpp
// Remove expired resources from the cache
cache.removeExpired();
```

### readFromJsonFile

Reads a resource from a JSON file asynchronously.

```cpp
void readFromJsonFile(const std::string &filePath, const std::function<T(const json &)> &fromJson);
```

#### Usage Example

```cpp
// Read a resource from a JSON file asynchronously and deserialize using a custom function
cache.readFromJsonFile("data.json", [](const json &jsonData) {
    // Custom deserialization from JSON logic
    return jsonData.get<T>();
});
```

### writeToJsonFile

Writes a resource to a JSON file asynchronously.

```cpp
void writeToJsonFile(const std::string &filePath, const std::function<json(const T &)> &toJson);
```

#### Usage Example

```cpp
// Write a resource to a JSON file asynchronously using a custom function
cache.writeToJsonFile("data.json", [](const T &data) {
    // Custom serialization to JSON logic
    json jsonData = data; // Assuming T is convertible to json
    return jsonData;
});
```

## Private Member Function

### evict

Evicts the oldest resource from the cache.

Note: This function is not intended for external use.

```cpp
void evict();
```
