# C++ Signal Classes Documentation

This document provides a detailed explanation of the signal classes and their usage as defined in the `atom::async` namespace.

## Table of Contents

1. [Signal](#signal)
2. [AsyncSignal](#asyncsignal)
3. [AutoDisconnectSignal](#autodisconnectsignal)
4. [ChainedSignal](#chainedsignal)
5. [TemplateSignal](#templatesignal)
6. [ThreadSafeSignal](#threadsafesignal)
7. [BroadcastSignal](#broadcastsignal)
8. [LimitedSignal](#limitedsignal)
9. [DynamicSignal](#dynamicsignal)
10. [ScopedSignal](#scopedsignal)

## Signal

### Overview

`Signal` is a basic signal class that allows connecting, disconnecting, and emitting slots.

### Template Parameters

- `Args`: The argument types for the slots.

### Public Methods

#### connect

```cpp
void connect(SlotType slot)
```

Connects a slot to the signal.

- **Parameters**:
  - `slot`: The slot to connect.

#### disconnect

```cpp
void disconnect(const SlotType& slot)
```

Disconnects a slot from the signal.

- **Parameters**:
  - `slot`: The slot to disconnect.

#### emit

```cpp
void emit(Args... args)
```

Emits the signal, calling all connected slots.

- **Parameters**:
  - `args`: The arguments to pass to the slots.

### Usage Example

```cpp
atom::async::Signal<int, std::string> mySignal;

auto slot1 = [](int x, const std::string& s) {
    std::cout << "Slot 1: " << x << ", " << s << std::endl;
};

auto slot2 = [](int x, const std::string& s) {
    std::cout << "Slot 2: " << x << ", " << s << std::endl;
};

mySignal.connect(slot1);
mySignal.connect(slot2);

mySignal.emit(42, "Hello, World!");

mySignal.disconnect(slot1);

mySignal.emit(10, "Goodbye!");
```

## AsyncSignal

### Overview

`AsyncSignal` is a signal class that allows asynchronous slot execution.

### Template Parameters

- `Args`: The argument types for the slots.

### Public Methods

#### connect

```cpp
void connect(SlotType slot)
```

Connects a slot to the signal.

- **Parameters**:
  - `slot`: The slot to connect.

#### disconnect

```cpp
void disconnect(const SlotType& slot)
```

Disconnects a slot from the signal.

- **Parameters**:
  - `slot`: The slot to disconnect.

#### emit

```cpp
void emit(Args... args)
```

Emits the signal asynchronously, calling all connected slots.

- **Parameters**:
  - `args`: The arguments to pass to the slots.

### Usage Example

```cpp
atom::async::AsyncSignal<int, std::string> myAsyncSignal;

auto slot1 = [](int x, const std::string& s) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Async Slot 1: " << x << ", " << s << std::endl;
};

auto slot2 = [](int x, const std::string& s) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Async Slot 2: " << x << ", " << s << std::endl;
};

myAsyncSignal.connect(slot1);
myAsyncSignal.connect(slot2);

myAsyncSignal.emit(42, "Hello, Async World!");
```

## AutoDisconnectSignal

### Overview

`AutoDisconnectSignal` is a signal class that allows automatic disconnection of slots using unique IDs.

### Template Parameters

- `Args`: The argument types for the slots.

### Public Methods

#### connect

```cpp
auto connect(SlotType slot) -> int
```

Connects a slot to the signal and returns its unique ID.

- **Parameters**:
  - `slot`: The slot to connect.
- **Returns**: The unique ID of the connected slot.

#### disconnect

```cpp
void disconnect(int id)
```

Disconnects a slot from the signal using its unique ID.

- **Parameters**:
  - `id`: The unique ID of the slot to disconnect.

#### emit

```cpp
void emit(Args... args)
```

Emits the signal, calling all connected slots.

- **Parameters**:
  - `args`: The arguments to pass to the slots.

### Usage Example

```cpp
atom::async::AutoDisconnectSignal<int, std::string> myAutoDisconnectSignal;

auto slot1 = [](int x, const std::string& s) {
    std::cout << "Auto Slot 1: " << x << ", " << s << std::endl;
};

auto slot2 = [](int x, const std::string& s) {
    std::cout << "Auto Slot 2: " << x << ", " << s << std::endl;
};

int id1 = myAutoDisconnectSignal.connect(slot1);
int id2 = myAutoDisconnectSignal.connect(slot2);

myAutoDisconnectSignal.emit(42, "Hello, Auto World!");

myAutoDisconnectSignal.disconnect(id1);

myAutoDisconnectSignal.emit(10, "Goodbye, Auto World!");
```

## ChainedSignal

### Overview

`ChainedSignal` is a signal class that allows chaining of signals.

### Template Parameters

- `Args`: The argument types for the slots.

### Public Methods

#### connect

```cpp
void connect(SlotType slot)
```

Connects a slot to the signal.

- **Parameters**:
  - `slot`: The slot to connect.

#### addChain

```cpp
void addChain(ChainedSignal<Args...>& nextSignal)
```

Adds a chained signal to be emitted after this signal.

- **Parameters**:
  - `nextSignal`: The next signal to chain.

#### emit

```cpp
void emit(Args... args)
```

Emits the signal, calling all connected slots and chained signals.

- **Parameters**:
  - `args`: The arguments to pass to the slots.

### Usage Example

```cpp
atom::async::ChainedSignal<int, std::string> signal1;
atom::async::ChainedSignal<int, std::string> signal2;

auto slot1 = [](int x, const std::string& s) {
    std::cout << "Chained Slot 1: " << x << ", " << s << std::endl;
};

auto slot2 = [](int x, const std::string& s) {
    std::cout << "Chained Slot 2: " << x << ", " << s << std::endl;
};

signal1.connect(slot1);
signal2.connect(slot2);

signal1.addChain(signal2);

signal1.emit(42, "Hello, Chained World!");
```

## TemplateSignal

### Overview

`TemplateSignal` is a signal class that allows connecting, disconnecting, and emitting slots.

### Template Parameters

- `Args`: The argument types for the slots.

### Public Methods

#### connect

```cpp
void connect(SlotType slot)
```

Connects a slot to the signal.

- **Parameters**:
  - `slot`: The slot to connect.

#### disconnect

```cpp
void disconnect(const SlotType& slot)
```

Disconnects a slot from the signal.

- **Parameters**:
  - `slot`: The slot to disconnect.

#### emit

```cpp
void emit(Args... args)
```

Emits the signal, calling all connected slots.

- **Parameters**:
  - `args`: The arguments to pass to the slots.

### Usage Example

```cpp
atom::async::TemplateSignal<int, std::string> myTemplateSignal;

auto slot1 = [](int x, const std::string& s) {
    std::cout << "Template Slot 1: " << x << ", " << s << std::endl;
};

auto slot2 = [](int x, const std::string& s) {
    std::cout << "Template Slot 2: " << x << ", " << s << std::endl;
};

myTemplateSignal.connect(slot1);
myTemplateSignal.connect(slot2);

myTemplateSignal.emit(42, "Hello, Template World!");

myTemplateSignal.disconnect(slot1);

myTemplateSignal.emit(10, "Goodbye, Template World!");
```

## ThreadSafeSignal

### Overview

`ThreadSafeSignal` is a signal class that ensures thread-safe slot execution.

### Template Parameters

- `Args`: The argument types for the slots.

### Public Methods

#### connect

```cpp
void connect(SlotType slot)
```

Connects a slot to the signal.

- **Parameters**:
  - `slot`: The slot to connect.

#### disconnect

```cpp
void disconnect(const SlotType& slot)
```

Disconnects a slot from the signal.

- **Parameters**:
  - `slot`: The slot to disconnect.

#### emit

```cpp
void emit(Args... args)
```

Emits the signal, calling all connected slots in a thread-safe manner.

- **Parameters**:
  - `args`: The arguments to pass to the slots.

### Usage Example

```cpp
atom::async::ThreadSafeSignal<int, std::string> myThreadSafeSignal;

auto slot1 = [](int x, const std::string& s) {
    std::cout << "Thread-safe Slot 1: " << x << ", " << s << std::endl;
};

auto slot2 = [](int x, const std::string& s) {
    std::cout << "Thread-safe Slot 2: " << x << ", " << s << std::endl;
};

myThreadSafeSignal.connect(slot1);
myThreadSafeSignal.connect(slot2);

std::thread t1([&]() { myThreadSafeSignal.emit(42, "Hello from Thread 1"); });
std::thread t2([&]() { myThreadSafeSignal.emit(10, "Hello from Thread 2"); });

t1.join();
t2.join();
```

## BroadcastSignal

### Overview

`BroadcastSignal` is a signal class that allows broadcasting to chained signals.

### Template Parameters

- `Args`: The argument types for the slots.

### Public Methods

#### connect

```cpp
void connect(SlotType slot)
```

Connects a slot to the signal.

- **Parameters**:
  - `slot`: The slot to connect.

#### disconnect

```cpp
void disconnect(const SlotType& slot)
```

Disconnects a slot from the signal.

- **Parameters**:
  - `slot`: The slot to disconnect.

#### emit

```cpp
void emit(Args... args)
```

Emits the signal, calling all connected slots and chained signals.

- **Parameters**:
  - `args`: The arguments to pass to the slots.

#### addChain

```cpp
void addChain(BroadcastSignal<Args...>& signal)
```

Adds a chained signal to be emitted after this signal.

- **Parameters**:
  - `signal`: The next signal to chain.

### Usage Example

```cpp
atom::async::BroadcastSignal<int, std::string> broadcastSignal1;
atom::async::BroadcastSignal<int, std::string> broadcastSignal2;

auto slot1 = [](int x, const std::string& s) {
    std::cout << "Broadcast Slot 1: " << x << ", " << s << std::endl;
};

auto slot2 = [](int x, const std::string& s) {
    std::cout << "Broadcast Slot 2: " << x << ", " << s << std::endl;
};

broadcastSignal1.connect(slot1);
broadcastSignal2.connect(slot2);

broadcastSignal1.addChain(broadcastSignal2);

broadcastSignal1.emit(42, "Hello, Broadcast World!");
```

## LimitedSignal

### Overview

`LimitedSignal` is a signal class that limits the number of times it can be emitted.

### Template Parameters

- `Args`: The argument types for the slots.

### Constructor

```cpp
explicit LimitedSignal(size_t maxCalls)
```

Constructs a new Limited Signal object.

- **Parameters**:
  - `maxCalls`: The maximum number of times the signal can be emitted.

### Public Methods

#### connect

```cpp
void connect(SlotType slot)
```

Connects a slot to the signal.

- **Parameters**:
  - `slot`: The slot to connect.

#### disconnect

```cpp
void disconnect(const SlotType& slot)
```

Disconnects a slot from the signal.

- **Parameters**:
  - `slot`: The slot to disconnect.

#### emit

```cpp
void emit(Args... args)
```

Emits the signal, calling all connected slots up to the maximum number of calls.

- **Parameters**:
  - `args`: The arguments to pass to the slots.

### Usage Example

```cpp
atom::async::LimitedSignal<int, std::string> limitedSignal(2);

auto slot = [](int x, const std::string& s) {
    std::cout << "Limited Slot: " << x << ", " << s << std::endl;
};

limitedSignal.connect(slot);

limitedSignal.emit(1, "First emission");
limitedSignal.emit(2, "Second emission");
limitedSignal.emit(3, "This won't be emitted");
```

## DynamicSignal

### Overview

`DynamicSignal` is a signal class that allows dynamic slot management.

### Template Parameters

- `Args`: The argument types for the slots.

### Public Methods

#### connect

```cpp
void connect(SlotType slot)
```

Connects a slot to the signal.

- **Parameters**:
  - `slot`: The slot to connect.

#### disconnect

```cpp
void disconnect(const SlotType& slot)
```

Disconnects a slot from the signal.

- **Parameters**:
  - `slot`: The slot to disconnect.

#### emit

```cpp
void emit(Args... args)
```

Emits the signal, calling all connected slots.

- **Parameters**:
  - `args`: The arguments to pass to the slots.

### Usage Example

```cpp
atom::async::DynamicSignal<int, std::string> dynamicSignal;

auto slot1 = [](int x, const std::string& s) {
    std::cout << "Dynamic Slot 1: " << x << ", " << s << std::endl;
};

auto slot2 = [](int x, const std::string& s) {
    std::cout << "Dynamic Slot 2: " << x << ", " << s << std::endl;
};

dynamicSignal.connect(slot1);
dynamicSignal.connect(slot2);

dynamicSignal.emit(42, "Hello, Dynamic World!");

dynamicSignal.disconnect(slot1);

dynamicSignal.emit(10, "Goodbye, Dynamic World!");
```

## ScopedSignal

### Overview

`ScopedSignal` is a signal class that allows scoped slot management using shared pointers.

### Template Parameters

- `Args`: The argument types for the slots.

### Public Methods

#### connect

```cpp
void connect(std::shared_ptr<SlotType> slotPtr)
```

Connects a slot to the signal using a shared pointer.

- **Parameters**:
  - `slotPtr`: The shared pointer to the slot to connect.

#### emit

```cpp
void emit(Args... args)
```

Emits the signal, calling all connected slots.

- **Parameters**:
  - `args`: The arguments to pass to the slots.

### Usage Example

```cpp
atom::async::ScopedSignal<int, std::string> scopedSignal;

auto slot1 = std::make_shared<std::function<void(int, const std::string&)>>(
    [](int x, const std::string& s) {
        std::cout << "Scoped Slot 1: " << x << ", " << s << std::endl;
    }
);

auto slot2 = std::make_shared<std::function<void(int, const std::string&)>>(
    [](int x, const std::string& s) {
        std::cout << "Scoped Slot 2: " << x << ", " << s << std::endl;
    }
);

scopedSignal.connect(slot1);
scopedSignal.connect(slot2);

scopedSignal.emit(42, "Hello, Scoped World!");

// The slot will be automatically disconnected when the shared_ptr is reset
slot1.reset();

scopedSignal.emit(10, "Goodbye, Scoped World!");
```

## General Notes on Usage

1. **Thread Safety**: Most of these signal classes are designed with thread safety in mind. However, it's important to note that the `Signal`, `ChainedSignal`, `TemplateSignal`, and `DynamicSignal` classes use a basic `std::mutex` for synchronization, which may not be suitable for all concurrent scenarios. The `ThreadSafeSignal` and `AsyncSignal` classes provide more robust thread-safety guarantees.

2. **Performance Considerations**: While these signal classes provide flexibility and safety, they may introduce some overhead due to synchronization mechanisms. For performance-critical applications, consider profiling and benchmarking to ensure they meet your requirements.

3. **Memory Management**: The `ScopedSignal` class uses `std::shared_ptr` for automatic memory management of slots. This can be particularly useful when dealing with objects that may be destroyed before the signal is emitted.

4. **Slot Disconnection**: Most signal classes provide a `disconnect` method to remove slots. It's important to disconnect slots when they are no longer needed to prevent memory leaks and unnecessary computations.

5. **Chaining and Broadcasting**: The `ChainedSignal` and `BroadcastSignal` classes allow for complex signal propagation patterns. Use these when you need to create hierarchical or networked signal structures.

6. **Limited Emissions**: The `LimitedSignal` class is useful for scenarios where you want to restrict the number of times a signal can be emitted. This can be helpful for implementing one-time notifications or limited-use events.

7. **Asynchronous Execution**: The `AsyncSignal` class provides asynchronous slot execution, which can be beneficial for long-running or I/O-bound operations. However, be cautious of potential race conditions and ensure proper synchronization when accessing shared resources from asynchronous slots.

8. **Template Usage**: All these signal classes are templated, allowing for type-safe slot connections and emissions. Ensure that the slot signatures match the signal's template parameters to avoid compilation errors.

9. **Error Handling**: These signal implementations do not inherently provide error handling for slot executions. Consider wrapping slot calls in try-catch blocks within your application code if exception handling is required.

10. **Scalability**: For applications with a large number of signals or slots, consider the memory and performance implications. The `DynamicSignal` class, for instance, allows for runtime management of slots, which can be useful in such scenarios.

By understanding these general principles and the specific behaviors of each signal class, you can effectively utilize these concurrent programming tools in your C++ applications. Choose the appropriate signal class based on your specific requirements for thread safety, performance, and functionality.
