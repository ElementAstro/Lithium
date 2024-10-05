# Trackable Class Template Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructor](#constructor)
4. [Public Methods](#public-methods)
5. [Operators](#operators)
6. [Usage Examples](#usage-examples)
7. [Best Practices and Considerations](#best-practices-and-considerations)

## Introduction

The `Trackable` class template is designed to create objects that can be observed for value changes. It provides a mechanism to notify observers when the value of the object changes, making it useful for implementing reactive programming patterns or data binding scenarios.

## Class Overview

```cpp
template <typename T>
class Trackable {
public:
    using value_type = T;

    explicit Trackable(T initialValue);

    void subscribe(std::function<void(const T&, const T&)> onChange);
    void setOnChangeCallback(std::function<void(const T&)> onChange);
    void unsubscribeAll();
    [[nodiscard]] auto hasSubscribers() const -> bool;
    [[nodiscard]] auto get() const -> const T&;
    [[nodiscard]] auto getTypeName() const -> std::string;

    auto operator=(T newValue) -> Trackable&;
    auto operator+=(const T& rhs) -> Trackable&;
    auto operator-=(const T& rhs) -> Trackable&;
    auto operator*=(const T& rhs) -> Trackable&;
    auto operator/=(const T& rhs) -> Trackable&;
    explicit operator T() const;

    void deferNotifications(bool defer);
    [[nodiscard]] auto deferScoped();

    // ... (private members)
};
```

## Constructor

```cpp
explicit Trackable(T initialValue);
```

Creates a new `Trackable` object with the given initial value.

Usage:

```cpp
Trackable<int> trackableInt(42);
Trackable<std::string> trackableString("Hello, World!");
```

## Public Methods

### subscribe

```cpp
void subscribe(std::function<void(const T&, const T&)> onChange);
```

Subscribes a callback function to be called when the value changes. The callback receives both the old and new values.

Usage:

```cpp
trackableInt.subscribe([](const int& oldValue, const int& newValue) {
    std::cout << "Value changed from " << oldValue << " to " << newValue << std::endl;
});
```

### setOnChangeCallback

```cpp
void setOnChangeCallback(std::function<void(const T&)> onChange);
```

Sets a callback function to be called when the value changes. The callback receives only the new value.

Usage:

```cpp
trackableString.setOnChangeCallback([](const std::string& newValue) {
    std::cout << "New value is: " << newValue << std::endl;
});
```

### unsubscribeAll

```cpp
void unsubscribeAll();
```

Removes all subscribed observers.

Usage:

```cpp
trackableInt.unsubscribeAll();
```

### hasSubscribers

```cpp
[[nodiscard]] auto hasSubscribers() const -> bool;
```

Checks if there are any subscribed observers.

Usage:

```cpp
if (trackableInt.hasSubscribers()) {
    std::cout << "trackableInt has subscribers" << std::endl;
}
```

### get

```cpp
[[nodiscard]] auto get() const -> const T&;
```

Returns the current value of the `Trackable` object.

Usage:

```cpp
int currentValue = trackableInt.get();
```

### getTypeName

```cpp
[[nodiscard]] auto getTypeName() const -> std::string;
```

Returns the demangled type name of the stored value.

Usage:

```cpp
std::cout << "Type of trackableInt: " << trackableInt.getTypeName() << std::endl;
```

### deferNotifications

```cpp
void deferNotifications(bool defer);
```

Controls whether notifications are deferred or not.

Usage:

```cpp
trackableInt.deferNotifications(true);
// Perform multiple operations...
trackableInt.deferNotifications(false);
```

### deferScoped

```cpp
[[nodiscard]] auto deferScoped();
```

Returns a scoped notification deferrer. Notifications are automatically re-enabled when the returned object is destroyed.

Usage:

```cpp
{
    auto defer = trackableInt.deferScoped();
    // Perform multiple operations...
} // Notifications are re-enabled here
```

## Operators

The `Trackable` class overloads several operators for convenient usage:

- Assignment operator (`=`)
- Compound assignment operators (`+=`, `-=`, `*=`, `/=`)
- Conversion operator to `T`

Usage:

```cpp
trackableInt = 100;
trackableInt += 5;
int value = static_cast<int>(trackableInt);
```

## Usage Examples

### Example 1: Basic Usage

```cpp
#include "trackable.hpp"
#include <iostream>

int main() {
    Trackable<int> counter(0);

    counter.subscribe([](const int& oldValue, const int& newValue) {
        std::cout << "Counter changed from " << oldValue << " to " << newValue << std::endl;
    });

    counter = 1;  // Triggers notification
    counter += 2; // Triggers notification
    counter *= 3; // Triggers notification

    std::cout << "Final value: " << counter.get() << std::endl;

    return 0;
}
```

Output:

```
Counter changed from 0 to 1
Counter changed from 1 to 3
Counter changed from 3 to 9
Final value: 9
```

### Example 2: Deferred Notifications

```cpp
#include "trackable.hpp"
#include <iostream>

int main() {
    Trackable<double> price(10.0);

    price.subscribe([](const double& oldPrice, const double& newPrice) {
        std::cout << "Price updated from $" << oldPrice << " to $" << newPrice << std::endl;
    });

    {
        auto defer = price.deferScoped();
        price = 11.0;  // No notification yet
        price *= 1.1;  // No notification yet
    } // Notification triggered here

    return 0;
}
```

Output:

```
Price updated from $10 to $12.1
```

### Example 3: Custom Type with Trackable

```cpp
#include "trackable.hpp"
#include <iostream>
#include <string>

struct Person {
    std::string name;
    int age;

    bool operator!=(const Person& other) const {
        return name != other.name || age != other.age;
    }
};

std::ostream& operator<<(std::ostream& os, const Person& p) {
    return os << p.name << " (" << p.age << ")";
}

int main() {
    Trackable<Person> person({"Alice", 30});

    person.subscribe([](const Person& oldPerson, const Person& newPerson) {
        std::cout << "Person changed from " << oldPerson << " to " << newPerson << std::endl;
    });

    person = {"Alice", 31};  // Triggers notification
    person = {"Bob", 25};    // Triggers notification

    std::cout << "Current person: " << person.get() << std::endl;
    std::cout << "Person type: " << person.getTypeName() << std::endl;

    return 0;
}
```

```
Person changed from Alice (30) to Alice (31)
Person changed from Alice (31) to Bob (25)
Current person: Bob (25)
Person type: Person
```

### Example 4: Thread-Safe Usage

The `Trackable` class is designed to be thread-safe. Here's an example demonstrating its use in a multi-threaded environment:

```cpp
#include "trackable.hpp"
#include <iostream>
#include <thread>
#include <vector>

int main() {
    Trackable<int> sharedCounter(0);

    sharedCounter.subscribe([](const int& oldValue, const int& newValue) {
        std::cout << "Counter changed from " << oldValue << " to " << newValue << std::endl;
    });

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&sharedCounter, i]() {
            for (int j = 0; j < 100; ++j) {
                sharedCounter += 1;
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Final counter value: " << sharedCounter.get() << std::endl;

    return 0;
}
```

This example creates multiple threads that increment a shared counter. The `Trackable` class ensures that all operations are thread-safe.

### Example 5: Using setOnChangeCallback

```cpp
#include "trackable.hpp"
#include <iostream>

int main() {
    Trackable<std::string> status("Idle");

    status.setOnChangeCallback([](const std::string& newStatus) {
        std::cout << "Status changed to: " << newStatus << std::endl;
    });

    status = "Running";
    status = "Paused";
    status = "Completed";

    return 0;
}
```

Output:

```
Status changed to: Running
Status changed to: Paused
Status changed to: Completed
```

### Example 6: Complex Data Structure with Trackable

```cpp
#include "trackable.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

struct DataPoint {
    double x, y;

    bool operator!=(const DataPoint& other) const {
        return x != other.x || y != other.y;
    }
};

class DataSet {
public:
    void addPoint(double x, double y) {
        points_.push_back({x, y});
    }

    const std::vector<DataPoint>& getPoints() const {
        return points_;
    }

    bool operator!=(const DataSet& other) const {
        return points_ != other.points_;
    }

private:
    std::vector<DataPoint> points_;
};

int main() {
    Trackable<DataSet> dataSet(DataSet{});

    dataSet.subscribe([](const DataSet& oldSet, const DataSet& newSet) {
        std::cout << "DataSet changed. Old size: " << oldSet.getPoints().size()
                  << ", New size: " << newSet.getPoints().size() << std::endl;
    });

    auto modifyDataSet = [](DataSet& ds) {
        ds.addPoint(1.0, 2.0);
        ds.addPoint(3.0, 4.0);
        return ds;
    };

    dataSet = modifyDataSet(dataSet.get());

    std::cout << "Final DataSet size: " << dataSet.get().getPoints().size() << std::endl;

    return 0;
}
```

Output:

```
DataSet changed. Old size: 0, New size: 2
Final DataSet size: 2
```

## Best Practices and Considerations

1. **Thread Safety**: The `Trackable` class is designed to be thread-safe. However, be cautious when performing operations that involve multiple calls to `Trackable` methods, as the overall operation may not be atomic.

2. **Performance**: While the `Trackable` class is efficient for most use cases, be mindful of the performance impact when dealing with a large number of observers or frequent value changes.

3. **Memory Management**: Ensure that any objects used in callbacks (e.g., lambdas capturing by reference) have a lifetime that exceeds that of the `Trackable` object.

4. **Exception Handling**: The `Trackable` class uses the `THROW_EXCEPTION` macro for error handling. Make sure to handle these exceptions appropriately in your application.

5. **Deferred Notifications**: Use the `deferNotifications` or `deferScoped` methods when you need to perform multiple operations on the `Trackable` object without triggering notifications for each change.

6. **Custom Types**: When using custom types with `Trackable`, ensure that the type implements the necessary operators (e.g., `!=`) for proper functionality.

7. **Avoid Circular Dependencies**: Be cautious when creating multiple `Trackable` objects that observe each other to avoid circular dependencies that could lead to infinite update loops.

8. **Callback Performance**: Keep in mind that callbacks are executed synchronously. Ensure that callback functions are efficient to avoid blocking the thread for extended periods.

9. **Type Safety**: Leverage the `getTypeName` method for debugging and ensuring type consistency, especially when dealing with template code.

10. **RAII for Deferred Notifications**: Use the `deferScoped` method to ensure that notifications are always re-enabled, even in the presence of exceptions.

11. **Const Correctness**: The `get` method returns a const reference to the value, ensuring that the value cannot be modified without going through the `Trackable` interface.

12. **Flexibility**: The `Trackable` class can be used with a wide variety of types, from simple built-in types to complex custom classes, making it versatile for different scenarios.

## Conclusion

The `Trackable` class template provides a powerful and flexible way to create observable objects in C++. By leveraging its features, you can implement reactive patterns, data binding, and other scenarios where you need to track and respond to value changes. Remember to consider the best practices and potential performance implications when using `Trackable` in your projects.
