# UDP Socket Hub Class Documentation

## Overview

The `UdpSocketHub` class is part of the `atom::connection` namespace and provides functionality for managing UDP (User Datagram Protocol) sockets and handling messages. This class allows you to create a UDP server that can send and receive messages over a network.

## Class Declaration

```cpp
namespace atom::connection {
    class UdpSocketHub {
        // ... (class members and methods)
    };
}
```

## Constructor and Destructor

### Constructor

```cpp
UdpSocketHub();
```

Creates a new instance of the `UdpSocketHub` class.

### Destructor

```cpp
~UdpSocketHub();
```

Destroys the `UdpSocketHub` instance and releases any resources.

## Public Methods

### start

```cpp
void start(int port);
```

Starts the UDP socket hub and binds it to the specified port.

- **Parameters:**
  - `port`: The port on which the UDP socket hub will listen for incoming messages.

#### Example:

```cpp
UdpSocketHub hub;
hub.start(8080);
std::cout << "UDP Socket Hub started on port 8080" << std::endl;
```

### stop

```cpp
void stop();
```

Stops the UDP socket hub.

#### Example:

```cpp
hub.stop();
std::cout << "UDP Socket Hub stopped" << std::endl;
```

### isRunning

```cpp
bool isRunning() const;
```

Checks if the UDP socket hub is currently running.

- **Returns:** `true` if the UDP socket hub is running, `false` otherwise.

#### Example:

```cpp
if (hub.isRunning()) {
    std::cout << "UDP Socket Hub is running" << std::endl;
} else {
    std::cout << "UDP Socket Hub is not running" << std::endl;
}
```

### addMessageHandler

```cpp
void addMessageHandler(MessageHandler handler);
```

Adds a message handler function to the UDP socket hub.

- **Parameters:**
  - `handler`: The message handler function to add. It should have the signature `void(const std::string&, const std::string&, int)`.

#### Example:

```cpp
hub.addMessageHandler([](const std::string& message, const std::string& ip, int port) {
    std::cout << "Received message from " << ip << ":" << port << ": " << message << std::endl;
});
```

### removeMessageHandler

```cpp
void removeMessageHandler(MessageHandler handler);
```

Removes a message handler function from the UDP socket hub.

- **Parameters:**
  - `handler`: The message handler function to remove.

#### Example:

```cpp
auto handler = [](const std::string& message, const std::string& ip, int port) {
    std::cout << "Received message from " << ip << ":" << port << ": " << message << std::endl;
};

hub.addMessageHandler(handler);
// ... later ...
hub.removeMessageHandler(handler);
```

### sendTo

```cpp
void sendTo(const std::string& message, const std::string& ip, int port);
```

Sends a message to the specified IP address and port.

- **Parameters:**
  - `message`: The message to send.
  - `ip`: The IP address of the recipient.
  - `port`: The port of the recipient.

#### Example:

```cpp
hub.sendTo("Hello, UDP!", "192.168.1.100", 8888);
std::cout << "Message sent to 192.168.1.100:8888" << std::endl;
```

## Usage Example

Here's a complete example demonstrating how to use the `UdpSocketHub` class:

```cpp
#include "udp_server.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    atom::connection::UdpSocketHub hub;

    // Add a message handler
    hub.addMessageHandler([](const std::string& message, const std::string& ip, int port) {
        std::cout << "Received message from " << ip << ":" << port << ": " << message << std::endl;

        // Echo the message back
        hub.sendTo("Echo: " + message, ip, port);
    });

    // Start the UDP socket hub
    hub.start(8080);
    std::cout << "UDP Socket Hub started on port 8080" << std::endl;

    // Keep the server running for a while
    std::this_thread::sleep_for(std::chrono::seconds(60));

    // Stop the UDP socket hub
    hub.stop();
    std::cout << "UDP Socket Hub stopped" << std::endl;

    return 0;
}
```

This example demonstrates how to:

1. Create a `UdpSocketHub` instance
2. Add a message handler that echoes received messages back to the sender
3. Start the UDP socket hub on port 8080
4. Keep the server running for 60 seconds
5. Stop the UDP socket hub

To test this server, you can use a UDP client to send messages to localhost (127.0.0.1) on port 8080. The server will echo back any messages it receives.

## Best Practices

1. **Error Handling**: Although not shown in the class interface, implement proper error handling in your application when using `UdpSocketHub`. Check for any exceptions that might be thrown during operations like starting the server or sending messages.

2. **Thread Safety**: If you're using the `UdpSocketHub` in a multi-threaded environment, ensure that access to shared resources is properly synchronized.

3. **Resource Management**: The `UdpSocketHub` class uses RAII (Resource Acquisition Is Initialization) through the use of `std::unique_ptr` for its implementation. This ensures that resources are properly cleaned up when the `UdpSocketHub` instance is destroyed.

4. **Message Handler Management**: Be careful when adding and removing message handlers. If you need to remove a specific handler, make sure to keep a reference to it when adding it.

5. **Graceful Shutdown**: Always call the `stop()` method before destroying the `UdpSocketHub` instance to ensure all resources are properly released and any ongoing operations are terminated cleanly.
