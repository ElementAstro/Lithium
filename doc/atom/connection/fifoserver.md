# FIFOServer Class Documentation

The `FIFOServer` class is part of the `atom::connection` namespace and provides functionality for managing a server that handles FIFO (First In, First Out) messages.

## Table of Contents

1. [Class Overview](#class-overview)
2. [Constructor](#constructor)
3. [Destructor](#destructor)
4. [Public Methods](#public-methods)
   - [sendMessage](#sendmessage)
   - [start](#start)
   - [stop](#stop)
   - [isRunning](#isrunning)
5. [Usage Examples](#usage-examples)

## Class Overview

```cpp
namespace atom::connection {

class FIFOServer {
public:
    explicit FIFOServer(std::string_view fifo_path);
    ~FIFOServer();

    void sendMessage(std::string message);
    void start();
    void stop();
    [[nodiscard]] bool isRunning() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace atom::connection
```

## Constructor

```cpp
explicit FIFOServer(std::string_view fifo_path);
```

Creates a new `FIFOServer` instance.

- **Parameters:**

  - `fifo_path`: A string view representing the path to the FIFO pipe.

- **Description:** This constructor initializes the FIFO server with the specified FIFO pipe path.

## Destructor

```cpp
~FIFOServer();
```

Destroys the `FIFOServer` instance.

- **Description:** Ensures that all resources are properly released and the FIFO server is stopped.

## Public Methods

### sendMessage

```cpp
void sendMessage(std::string message);
```

Sends a message through the FIFO pipe.

- **Parameters:**

  - `message`: The message to be sent (as a string).

- **Description:** This method sends the specified message through the FIFO pipe.

### start

```cpp
void start();
```

Starts the FIFO server.

- **Description:** Initiates the server, allowing it to handle incoming messages.

### stop

```cpp
void stop();
```

Stops the FIFO server.

- **Description:** Halts the server, preventing it from handling further messages.

### isRunning

```cpp
[[nodiscard]] bool isRunning() const;
```

Checks if the server is currently running.

- **Returns:** `true` if the server is running, `false` otherwise.

- **Description:** Used to determine the current state of the FIFO server.

## Usage Examples

Here are some examples demonstrating how to use the `FIFOServer` class:

### Creating and Starting a FIFOServer

```cpp
#include "fifoserver.hpp"
#include <iostream>

int main() {
    atom::connection::FIFOServer server("/tmp/myfifo");

    std::cout << "Starting FIFO server..." << std::endl;
    server.start();

    if (server.isRunning()) {
        std::cout << "FIFO server is running." << std::endl;
    } else {
        std::cerr << "Failed to start FIFO server." << std::endl;
        return 1;
    }

    // Keep the server running
    std::cout << "Press Enter to stop the server..." << std::endl;
    std::cin.get();

    server.stop();
    std::cout << "FIFO server stopped." << std::endl;

    return 0;
}
```

### Sending Messages with FIFOServer

```cpp
#include "fifoserver.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    atom::connection::FIFOServer server("/tmp/myfifo");

    server.start();

    if (server.isRunning()) {
        std::cout << "FIFO server is running. Sending messages..." << std::endl;

        for (int i = 1; i <= 5; ++i) {
            std::string message = "Message " + std::to_string(i);
            server.sendMessage(message);
            std::cout << "Sent: " << message << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        server.stop();
        std::cout << "FIFO server stopped." << std::endl;
    } else {
        std::cerr << "Failed to start FIFO server." << std::endl;
        return 1;
    }

    return 0;
}
```

### Using FIFOServer in a Multi-threaded Environment

```cpp
#include "fifoserver.hpp"
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

std::atomic<bool> running(true);

void serverThread(atom::connection::FIFOServer& server) {
    server.start();

    while (running && server.isRunning()) {
        // Simulating server work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    server.stop();
}

int main() {
    atom::connection::FIFOServer server("/tmp/myfifo");

    std::thread t(serverThread, std::ref(server));

    // Main thread sends messages
    for (int i = 1; i <= 10; ++i) {
        std::string message = "Message " + std::to_string(i);
        server.sendMessage(message);
        std::cout << "Sent: " << message << std::endl;

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    running = false;
    t.join();

    std::cout << "FIFO server stopped." << std::endl;

    return 0;
}
```
