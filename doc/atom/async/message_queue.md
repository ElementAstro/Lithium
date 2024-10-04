# Documentation for message_queue.hpp

This document provides a detailed overview of the `message_queue.hpp` header file, which contains the `MessageQueue` class template for implementing a message queue system with Asio support in C++.

## Table of Contents

1. [MessageQueue Class Template](#messagequeue-class-template)
2. [Constructor](#constructor)
3. [Public Methods](#public-methods)
4. [Private Methods](#private-methods)
5. [Usage Examples](#usage-examples)

## MessageQueue Class Template

The `MessageQueue` class template provides a flexible message queue system with support for asynchronous operations using Asio. It allows subscribers to receive messages of a specified type `T`.

### Key Features

- Type-safe message publishing and subscribing
- Priority-based message handling
- Subscriber filtering
- Timeout handling for message processing
- Asynchronous message processing using Asio

## Constructor

```cpp
explicit MessageQueue(asio::io_context& ioContext);
```

Constructs a `MessageQueue` instance with the given Asio `io_context`.

## Public Methods

### subscribe

```cpp
void subscribe(CallbackType callback, const std::string& subscriberName,
               int priority = 0, FilterType filter = nullptr,
               std::chrono::milliseconds timeout = std::chrono::milliseconds::zero());
```

Subscribes to messages with a callback and optional filter and timeout.

- `callback`: The function to be called when a new message is received.
- `subscriberName`: The name of the subscriber.
- `priority`: The priority of the subscriber (higher priority receives messages first).
- `filter`: An optional filter to only receive messages that match the criteria.
- `timeout`: The maximum time allowed for the subscriber to process a message.

### unsubscribe

```cpp
void unsubscribe(CallbackType callback);
```

Unsubscribes from messages using the given callback.

### publish

```cpp
void publish(const T& message, int priority = 0);
```

Publishes a message to the queue, with an optional priority.

### startProcessing

```cpp
void startProcessing();
```

Starts processing messages in the queue.

### stopProcessing

```cpp
void stopProcessing();
```

Stops processing messages in the queue.

### getMessageCount

```cpp
auto getMessageCount() const -> size_t;
```

Gets the number of messages currently in the queue.

### getSubscriberCount

```cpp
auto getSubscriberCount() const -> size_t;
```

Gets the number of subscribers currently subscribed to the queue.

### cancelMessages

```cpp
void cancelMessages(std::function<bool(const T&)> cancelCondition);
```

Cancels specific messages that meet a given condition.

## Private Methods

### processMessages

```cpp
void processMessages();
```

Processes messages in the queue.

### applyFilter

```cpp
bool applyFilter(const Subscriber& subscriber, const T& message);
```

Applies the filter to a message for a given subscriber.

### handleTimeout

```cpp
bool handleTimeout(const Subscriber& subscriber, const T& message);
```

Handles the timeout for a given subscriber and message.

## Usage Examples

Here are some examples demonstrating how to use the `MessageQueue` class:

### Basic Usage

```cpp
#include "message_queue.hpp"
#include <iostream>
#include <string>

int main() {
    asio::io_context io_context;
    atom::async::MessageQueue<std::string> messageQueue(io_context);

    // Subscribe to messages
    messageQueue.subscribe([](const std::string& msg) {
        std::cout << "Received: " << msg << std::endl;
    }, "Subscriber1");

    // Publish a message
    messageQueue.publish("Hello, World!");

    // Start processing messages
    messageQueue.startProcessing();

    // Run the io_context
    io_context.run();

    return 0;
}
```

### Using Priorities and Filters

```cpp
auto highPriorityFilter = [](const std::string& msg) {
    return msg.find("URGENT") != std::string::npos;
};

messageQueue.subscribe([](const std::string& msg) {
    std::cout << "High priority message: " << msg << std::endl;
}, "HighPrioritySubscriber", 10, highPriorityFilter);

messageQueue.subscribe([](const std::string& msg) {
    std::cout << "Normal message: " << msg << std::endl;
}, "NormalSubscriber", 0);

messageQueue.publish("URGENT: System failure", 10);
messageQueue.publish("Regular update", 0);
```

### Using Timeouts

```cpp
auto slowSubscriber = [](const std::string& msg) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Slow subscriber processed: " << msg << std::endl;
};

messageQueue.subscribe(slowSubscriber, "SlowSubscriber", 0, nullptr, std::chrono::seconds(1));

messageQueue.publish("This message might timeout");
```

### Cancelling Messages

```cpp
messageQueue.publish("Message 1");
messageQueue.publish("Message 2");
messageQueue.publish("Message 3");

messageQueue.cancelMessages([](const std::string& msg) {
    return msg == "Message 2";
});

std::cout << "Messages in queue: " << messageQueue.getMessageCount() << std::endl;
```

### Unsubscribing

```cpp
auto callback = [](const std::string& msg) {
    std::cout << "Temporary subscriber: " << msg << std::endl;
};

messageQueue.subscribe(callback, "TemporarySubscriber");
messageQueue.publish("This will be received");

messageQueue.unsubscribe(callback);
messageQueue.publish("This will not be received by the temporary subscriber");
```

These examples demonstrate the basic usage of the `MessageQueue` class, including subscribing to messages, publishing messages, using priorities and filters, handling timeouts, cancelling messages, and unsubscribing. Remember to handle exceptions and consider thread safety when using the `MessageQueue` in a multi-threaded environment.
