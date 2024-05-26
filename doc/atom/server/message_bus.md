# MessageBus Class Documentation

The `MessageBus` class provides a simple message bus implementation for handling message subscriptions, publishing, and processing.

## Constructor

### Example

```cpp
MessageBus bus(500); // Create a MessageBus object with a maximum queue size of 500
```

## Common Methods

### createShared()

Creates and returns a shared pointer to a new `MessageBus` object.

#### Example

```cpp
auto sharedBus = MessageBus::createShared();
```

### createUnique()

Creates and returns a unique pointer to a new `MessageBus` object.

#### Example

```cpp
auto uniqueBus = MessageBus::createUnique();
```

## MessageBus Methods

### Subscribe()

Subscribes a callback function to a specific topic.

#### Example

```cpp
bus.Subscribe<int>("topic_name", [](const int& message) {
    std::cout << "Received message: " << message << std::endl;
});
```

### Publish()

Publishes a message to a specific topic.

#### Example

```cpp
bus.Publish<int>("topic_name", 42);
```

_Expected Output_: "Published message to topic: topic_name"

### TryReceive()

Tries to receive a message with a timeout.

#### Example

```cpp
int receivedMessage;
if (bus.TryReceive(receivedMessage)) {
    std::cout << "Received message: " << receivedMessage << std::endl;
}
```

### StartProcessingThread()

Starts a processing thread for a specific message type.

#### Example

```cpp
bus.StartProcessingThread<int>();
```

### StopProcessingThread()

Stops the processing thread for a specific message type.

#### Example

```cpp
bus.StopProcessingThread<int>();
```

### StopAllProcessingThreads()

Stops all processing threads.

#### Example

```cpp
bus.StopAllProcessingThreads();
```

### GlobalSubscribe()

Subscribes a global callback function that will be triggered for all message types.

#### Example

```cpp
bus.GlobalSubscribe<int>([](const int& message) {
    std::cout << "Global message received: " << message << std::endl;
});
```

### Unsubscribe()

Unsubscribes a callback function from a specific topic.

#### Example

```cpp
bus.Unsubscribe<int>("topic_name", callback_function);
```

### UnsubscribeFromNamespace()

Unsubscribes a callback function from a specific namespace.

#### Example

```cpp
bus.UnsubscribeFromNamespace<int>("namespace_name", callback_function);
```

### UnsubscribeAll()

Unsubscribes from all topics within a namespace.

#### Example

```cpp
bus.UnsubscribeAll("namespace_name");
```

### GlobalUnsubscribe()

Unsubscribes a global callback function.

#### Example

```cpp
bus.GlobalUnsubscribe<int>(callback_function);
```

### TryPublish()

Tries to publish a message with a timeout.

#### Example

```cpp
bus.TryPublish<int>("topic_name", 42);
```
