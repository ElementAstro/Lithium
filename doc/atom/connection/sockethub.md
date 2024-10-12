# SocketHub Class Documentation

The `SocketHub` class is part of the `atom::connection` namespace and provides functionality for managing socket connections. It allows for starting and stopping a socket service, handling multiple client connections, and processing incoming messages through custom handlers.

## Table of Contents

1. [Class Overview](#class-overview)
2. [Constructor and Destructor](#constructor-and-destructor)
3. [Public Methods](#public-methods)
   - [start](#start)
   - [stop](#stop)
   - [addHandler](#addhandler)
   - [isRunning](#isrunning)
4. [Usage Examples](#usage-examples)

## Class Overview

```cpp
namespace atom::connection {

class SocketHub {
public:
    SocketHub();
    ~SocketHub();

    void start(int port);
    void stop();
    void addHandler(std::function<void(std::string)> handler);
    [[nodiscard]] auto isRunning() const -> bool;

private:
    std::unique_ptr<SocketHubImpl> impl_;
};

}  // namespace atom::connection
```

## Constructor and Destructor

### Constructor

```cpp
SocketHub();
```

Creates a new `SocketHub` instance.

### Destructor

```cpp
~SocketHub();
```

Destroys the `SocketHub` instance, cleaning up resources and stopping any ongoing socket operations.

## Public Methods

### start

```cpp
void start(int port);
```

Starts the socket service.

- **Parameters:**

  - `port`: The port number on which the socket service will listen.

- **Description:** Initializes the socket service and starts listening for incoming connections on the specified port. It spawns threads to handle each connected client.

### stop

```cpp
void stop();
```

Stops the socket service.

- **Description:** Shuts down the socket service, closes all client connections, and stops any running threads associated with handling client messages.

### addHandler

```cpp
void addHandler(std::function<void(std::string)> handler);
```

Adds a message handler.

- **Parameters:**

  - `handler`: A function to handle incoming messages from clients.

- **Description:** The provided handler function will be called with the received message as a string parameter. Multiple handlers can be added and will be called in the order they are added.

### isRunning

```cpp
[[nodiscard]] auto isRunning() const -> bool;
```

Checks if the socket service is currently running.

- **Returns:** `true` if the socket service is running, `false` otherwise.

- **Description:** This method returns the status of the socket service, indicating whether it is currently active and listening for connections.

## Usage Examples

Here are some examples demonstrating how to use the `SocketHub` class:

### Basic Usage: Starting and Stopping the SocketHub

```cpp
#include "sockethub.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    atom::connection::SocketHub hub;

    // Start the socket service on port 8080
    hub.start(8080);

    if (hub.isRunning()) {
        std::cout << "SocketHub is running on port 8080" << std::endl;

        // Keep the service running for 1 minute
        std::this_thread::sleep_for(std::chrono::minutes(1));

        // Stop the service
        hub.stop();
        std::cout << "SocketHub has been stopped" << std::endl;
    } else {
        std::cerr << "Failed to start SocketHub" << std::endl;
    }

    return 0;
}
```

### Adding a Message Handler

```cpp
#include "sockethub.hpp"
#include <iostream>
#include <thread>
#include <chrono>

void messageHandler(const std::string& message) {
    std::cout << "Received message: " << message << std::endl;
}

int main() {
    atom::connection::SocketHub hub;

    // Add a message handler
    hub.addHandler(messageHandler);

    // Start the socket service on port 8080
    hub.start(8080);

    if (hub.isRunning()) {
        std::cout << "SocketHub is running on port 8080" << std::endl;

        // Keep the service running for 5 minutes
        std::this_thread::sleep_for(std::chrono::minutes(5));

        // Stop the service
        hub.stop();
        std::cout << "SocketHub has been stopped" << std::endl;
    } else {
        std::cerr << "Failed to start SocketHub" << std::endl;
    }

    return 0;
}
```

### Using Multiple Handlers and Lambda Functions

```cpp
#include "sockethub.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    atom::connection::SocketHub hub;

    // Add multiple handlers
    hub.addHandler([](const std::string& message) {
        std::cout << "Handler 1: " << message << std::endl;
    });

    hub.addHandler([](const std::string& message) {
        std::cout << "Handler 2: " << message.length() << " characters" << std::endl;
    });

    // Start the socket service on port 8080
    hub.start(8080);

    if (hub.isRunning()) {
        std::cout << "SocketHub is running on port 8080" << std::endl;

        // Keep the service running for 10 minutes
        std::this_thread::sleep_for(std::chrono::minutes(10));

        // Stop the service
        hub.stop();
        std::cout << "SocketHub has been stopped" << std::endl;
    } else {
        std::cerr << "Failed to start SocketHub" << std::endl;
    }

    return 0;
}
```
