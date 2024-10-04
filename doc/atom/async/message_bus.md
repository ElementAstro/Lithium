# Documentation for message_bus.hpp

This document provides a detailed overview of the `message_bus.hpp` header file, which contains the `MessageBus` class for implementing a message bus system with Asio support in C++.

## Table of Contents

1. [MessageBus Class](#messagebus-class)
2. [Constructor and Static Methods](#constructor-and-static-methods)
3. [Publishing Methods](#publishing-methods)
4. [Subscription Methods](#subscription-methods)
5. [Unsubscription Methods](#unsubscription-methods)
6. [Query Methods](#query-methods)
7. [Utility Methods](#utility-methods)
8. [Usage Examples](#usage-examples)

## MessageBus Class

The `MessageBus` class provides a flexible message bus system with support for asynchronous operations using Asio.

### Key Features

- Type-safe message publishing and subscribing
- Support for delayed message publishing
- Namespace-based message routing
- Asynchronous and synchronous message handling
- Message filtering
- Message history tracking

## Constructor and Static Methods

### Constructor

```cpp
explicit MessageBus(asio::io_context& io_context);
```

Constructs a `MessageBus` instance with the given Asio `io_context`.

### Static Factory Method

```cpp
static auto createShared(asio::io_context& io_context) -> std::shared_ptr<MessageBus>;
```

Creates and returns a shared pointer to a new `MessageBus` instance.

## Publishing Methods

### publish

```cpp
template <typename MessageType>
void publish(const std::string& name, const MessageType& message,
             std::optional<std::chrono::milliseconds> delay = std::nullopt);
```

Publishes a message to the bus, optionally with a delay.

### publishGlobal

```cpp
template <typename MessageType>
void publishGlobal(const MessageType& message);
```

Publishes a message to all subscribers globally.

## Subscription Methods

### subscribe

```cpp
template <typename MessageType>
auto subscribe(const std::string& name,
               std::function<void(const MessageType&)> handler,
               bool async = true,
               bool once = false,
               std::function<bool(const MessageType&)> filter = [](const MessageType&) { return true; }) -> Token;
```

Subscribes to a message with the given name and handler function.

- `name`: The name of the message to subscribe to.
- `handler`: The function to be called when a message is received.
- `async`: Whether to call the handler asynchronously (default: true).
- `once`: Whether to unsubscribe after the first message is received (default: false).
- `filter`: Optional function to filter messages (default: accept all messages).

Returns a token that can be used to unsubscribe later.

## Unsubscription Methods

### unsubscribe

```cpp
template <typename MessageType>
void unsubscribe(Token token);
```

Unsubscribes from a message using the given token.

### unsubscribeAll

```cpp
template <typename MessageType>
void unsubscribeAll(const std::string& name);
```

Unsubscribes all handlers for a given message name.

### clearAllSubscribers

```cpp
void clearAllSubscribers();
```

Clears all subscribers from the message bus.

## Query Methods

### getSubscriberCount

```cpp
template <typename MessageType>
auto getSubscriberCount(const std::string& name) -> std::size_t;
```

Gets the number of subscribers for a given message name.

### getNamespaceSubscriberCount

```cpp
template <typename MessageType>
auto getNamespaceSubscriberCount(const std::string& namespaceName) -> std::size_t;
```

Gets the number of subscribers for a given namespace.

### hasSubscriber

```cpp
template <typename MessageType>
auto hasSubscriber(const std::string& name) -> bool;
```

Checks if there are any subscribers for a given message name.

### getActiveNamespaces

```cpp
auto getActiveNamespaces() const -> std::vector<std::string>;
```

Gets the list of active namespaces.

## Utility Methods

### getMessageHistory

```cpp
template <typename MessageType>
auto getMessageHistory(const std::string& name) const -> std::vector<MessageType>;
```

Gets the message history for a given message name.

## Usage Examples

Here are some examples demonstrating how to use the `MessageBus` class:

### Basic Usage

```cpp
#include "message_bus.hpp"
#include <iostream>
#include <string>

int main() {
    asio::io_context io_context;
    auto messageBus = atom::async::MessageBus::createShared(io_context);

    // Subscribe to a message
    auto token = messageBus->subscribe<std::string>("greeting", [](const std::string& msg) {
        std::cout << "Received: " << msg << std::endl;
    });

    // Publish a message
    messageBus->publish("greeting", std::string("Hello, World!"));

    // Run the io_context
    io_context.run();

    // Unsubscribe
    messageBus->unsubscribe<std::string>(token);

    return 0;
}
```

### Delayed Publishing

```cpp
messageBus->publish("delayed_message", std::string("This message is delayed"),
                    std::chrono::milliseconds(1000));
```

### Using Filters

```cpp
messageBus->subscribe<int>("number", [](int num) {
    std::cout << "Received even number: " << num << std::endl;
}, true, false, [](int num) { return num % 2 == 0; });

messageBus->publish("number", 1);  // Will not be received
messageBus->publish("number", 2);  // Will be received
```

### Namespace-based Routing

```cpp
messageBus->subscribe<std::string>("system.log", [](const std::string& log) {
    std::cout << "System log: " << log << std::endl;
});

messageBus->subscribe<std::string>("system.error", [](const std::string& error) {
    std::cerr << "System error: " << error << std::endl;
});

messageBus->publish("system.log", std::string("Application started"));
messageBus->publish("system.error", std::string("Connection failed"));
```

### Querying Message Bus State

```cpp
auto subscriberCount = messageBus->getSubscriberCount<std::string>("greeting");
auto namespaceCount = messageBus->getNamespaceSubscriberCount<std::string>("system");
auto activeNamespaces = messageBus->getActiveNamespaces();

std::cout << "Subscriber count for 'greeting': " << subscriberCount << std::endl;
std::cout << "Subscriber count for 'system' namespace: " << namespaceCount << std::endl;
std::cout << "Active namespaces: ";
for (const auto& ns : activeNamespaces) {
    std::cout << ns << " ";
}
std::cout << std::endl;
```

These examples demonstrate the basic usage of the `MessageBus` class, including subscribing to messages, publishing messages, using filters, namespace-based routing, and querying the message bus state. Remember to handle exceptions and consider thread safety when using the `MessageBus` in a multi-threaded environment.
