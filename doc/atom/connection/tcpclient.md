# TcpClient Class Documentation

The `TcpClient` class is part of the `atom::connection` namespace and provides functionality for creating and managing TCP client connections. This class allows users to connect to TCP servers, send and receive data, and handle various events related to the connection.

## Table of Contents

1. [Class Overview](#class-overview)
2. [Constructor and Destructor](#constructor-and-destructor)
3. [Public Methods](#public-methods)
   - [connect](#connect)
   - [disconnect](#disconnect)
   - [send](#send)
   - [receive](#receive)
   - [isConnected](#isconnected)
   - [getErrorMessage](#geterrormessage)
   - [setOnConnectedCallback](#setonconnectedcallback)
   - [setOnDisconnectedCallback](#setondisconnectedcallback)
   - [setOnDataReceivedCallback](#setondatareceivedcallback)
   - [setOnErrorCallback](#setonerrorcallback)
   - [startReceiving](#startreceiving)
   - [stopReceiving](#stopreceiving)
4. [Callback Types](#callback-types)
5. [Usage Examples](#usage-examples)

## Class Overview

```cpp
namespace atom::connection {

class TcpClient : public NonCopyable {
public:
    TcpClient();
    ~TcpClient() override;

    // ... (public methods)

private:
    class Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace atom::connection
```

## Constructor and Destructor

### Constructor

```cpp
TcpClient();
```

Creates a new `TcpClient` instance.

### Destructor

```cpp
~TcpClient() override;
```

Destroys the `TcpClient` instance and cleans up resources.

## Public Methods

### connect

```cpp
auto connect(const std::string& host, int port,
             std::chrono::milliseconds timeout = std::chrono::milliseconds::zero()) -> bool;
```

Connects to a TCP server.

- **Parameters:**
  - `host`: The hostname or IP address of the server.
  - `port`: The port number of the server.
  - `timeout`: The connection timeout duration (optional).
- **Returns:** `true` if the connection is successful, `false` otherwise.

### disconnect

```cpp
void disconnect();
```

Disconnects from the server.

### send

```cpp
auto send(const std::vector<char>& data) -> bool;
```

Sends data to the server.

- **Parameters:**
  - `data`: The data to be sent.
- **Returns:** `true` if the data is sent successfully, `false` otherwise.

### receive

```cpp
auto receive(size_t size, std::chrono::milliseconds timeout = std::chrono::milliseconds::zero())
    -> std::future<std::vector<char>>;
```

Receives data from the server.

- **Parameters:**
  - `size`: The number of bytes to receive.
  - `timeout`: The receive timeout duration (optional).
- **Returns:** A `std::future` containing the received data.

### isConnected

```cpp
[[nodiscard]] auto isConnected() const -> bool;
```

Checks if the client is connected to the server.

- **Returns:** `true` if connected, `false` otherwise.

### getErrorMessage

```cpp
[[nodiscard]] auto getErrorMessage() const -> std::string;
```

Gets the error message in case of any error.

- **Returns:** The error message.

### setOnConnectedCallback

```cpp
void setOnConnectedCallback(const OnConnectedCallback& callback);
```

Sets the callback function to be called when connected to the server.

- **Parameters:**
  - `callback`: The callback function.

### setOnDisconnectedCallback

```cpp
void setOnDisconnectedCallback(const OnDisconnectedCallback& callback);
```

Sets the callback function to be called when disconnected from the server.

- **Parameters:**
  - `callback`: The callback function.

### setOnDataReceivedCallback

```cpp
void setOnDataReceivedCallback(const OnDataReceivedCallback& callback);
```

Sets the callback function to be called when data is received from the server.

- **Parameters:**
  - `callback`: The callback function.

### setOnErrorCallback

```cpp
void setOnErrorCallback(const OnErrorCallback& callback);
```

Sets the callback function to be called when an error occurs.

- **Parameters:**
  - `callback`: The callback function.

### startReceiving

```cpp
void startReceiving(size_t bufferSize);
```

Starts receiving data from the server.

- **Parameters:**
  - `bufferSize`: The size of the receive buffer.

### stopReceiving

```cpp
void stopReceiving();
```

Stops receiving data from the server.

## Callback Types

The `TcpClient` class defines several callback function types:

```cpp
using OnConnectedCallback = std::function<void()>;
using OnDisconnectedCallback = std::function<void()>;
using OnDataReceivedCallback = std::function<void(const std::vector<char>&)>;
using OnErrorCallback = std::function<void(const std::string&)>;
```

These callback types are used to handle various events in the TCP client.

## Usage Examples

Here are some examples demonstrating how to use the `TcpClient` class:

### Basic Connection and Data Sending

```cpp
#include "tcpclient.hpp"
#include <iostream>
#include <vector>

int main() {
    atom::connection::TcpClient client;

    if (client.connect("example.com", 80)) {
        std::cout << "Connected to server" << std::endl;

        std::vector<char> data = {'H', 'e', 'l', 'l', 'o'};
        if (client.send(data)) {
            std::cout << "Data sent successfully" << std::endl;
        } else {
            std::cerr << "Failed to send data: " << client.getErrorMessage() << std::endl;
        }

        client.disconnect();
    } else {
        std::cerr << "Failed to connect: " << client.getErrorMessage() << std::endl;
    }

    return 0;
}
```

### Using Callbacks

```cpp
#include "tcpclient.hpp"
#include <iostream>
#include <vector>

void onConnected() {
    std::cout << "Connected to server" << std::endl;
}

void onDisconnected() {
    std::cout << "Disconnected from server" << std::endl;
}

void onDataReceived(const std::vector<char>& data) {
    std::cout << "Received data: " << std::string(data.begin(), data.end()) << std::endl;
}

void onError(const std::string& errorMessage) {
    std::cerr << "Error: " << errorMessage << std::endl;
}

int main() {
    atom::connection::TcpClient client;

    client.setOnConnectedCallback(onConnected);
    client.setOnDisconnectedCallback(onDisconnected);
    client.setOnDataReceivedCallback(onDataReceived);
    client.setOnErrorCallback(onError);

    if (client.connect("example.com", 80)) {
        client.startReceiving(1024);  // Start receiving with a 1KB buffer

        // Keep the connection open for a while
        std::this_thread::sleep_for(std::chrono::seconds(10));

        client.stopReceiving();
        client.disconnect();
    }

    return 0;
}
```

### Asynchronous Data Receiving

```cpp
#include "tcpclient.hpp"
#include <iostream>
#include <vector>
#include <future>

int main() {
    atom::connection::TcpClient client;

    if (client.connect("example.com", 80)) {
        std::cout << "Connected to server" << std::endl;

        // Send a request
        std::vector<char> request = {'G', 'E', 'T', ' ', '/', '\r', '\n', '\r', '\n'};
        client.send(request);

        // Do some other work while waiting for the response
        std::cout << "Waiting for response..." << std::endl;

        // Wait for the response
        try {
            auto response = futureResponse.get();
            std::cout << "Received response: " << std::string(response.begin(), response.end()) << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error receiving data: " << e.what() << std::endl;
        }

        client.disconnect();
    } else {
        std::cerr << "Failed to connect: " << client.getErrorMessage() << std::endl;
    }

    return 0;
}
```

### Implementing a Simple Echo Client

```cpp
#include "tcpclient.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <thread>

class EchoClient {
public:
    EchoClient() {
        client_.setOnConnectedCallback([this]() { onConnected(); });
        client_.setOnDisconnectedCallback([this]() { onDisconnected(); });
        client_.setOnDataReceivedCallback([this](const std::vector<char>& data) { onDataReceived(data); });
        client_.setOnErrorCallback([this](const std::string& error) { onError(error); });
    }

    void start(const std::string& host, int port) {
        if (client_.connect(host, port)) {
            client_.startReceiving(1024);
            runInputLoop();
        } else {
            std::cerr << "Failed to connect: " << client_.getErrorMessage() << std::endl;
        }
    }

private:
    void onConnected() {
        std::cout << "Connected to server. Type your messages (or 'quit' to exit):" << std::endl;
    }

    void onDisconnected() {
        std::cout << "Disconnected from server." << std::endl;
    }

    void onDataReceived(const std::vector<char>& data) {
        std::cout << "Server echo: " << std::string(data.begin(), data.end()) << std::endl;
    }

    void onError(const std::string& error) {
        std::cerr << "Error: " << error << std::endl;
    }

    void runInputLoop() {
        std::string input;
        while (client_.isConnected()) {
            std::getline(std::cin, input);
            if (input == "quit") {
                break;
            }
            std::vector<char> data(input.begin(), input.end());
            if (!client_.send(data)) {
                std::cerr << "Failed to send data: " << client_.getErrorMessage() << std::endl;
            }
        }
        client_.stopReceiving();
        client_.disconnect();
    }

    atom::connection::TcpClient client_;
};

int main() {
    EchoClient echoClient;
    echoClient.start("localhost", 12345);
    return 0;
}
```

### Implementing a Simple HTTP Client

```cpp
#include "tcpclient.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class SimpleHttpClient {
public:
    SimpleHttpClient(const std::string& host, int port = 80)
        : host_(host), port_(port) {}

    std::string get(const std::string& path) {
        atom::connection::TcpClient client;

        if (!client.connect(host_, port_)) {
            throw std::runtime_error("Failed to connect: " + client.getErrorMessage());
        }

        std::ostringstream requestStream;
        requestStream << "GET " << path << " HTTP/1.1\r\n"
                      << "Host: " << host_ << "\r\n"
                      << "Connection: close\r\n\r\n";

        std::string request = requestStream.str();
        std::vector<char> requestData(request.begin(), request.end());

        if (!client.send(requestData)) {
            throw std::runtime_error("Failed to send request: " + client.getErrorMessage());
        }

        std::vector<char> responseData;
        auto futureResponse = client.receive(4096);  // Receive up to 4KB

        try {
            responseData = futureResponse.get();
        } catch (const std::exception& e) {
            throw std::runtime_error("Error receiving data: " + std::string(e.what()));
        }

        client.disconnect();

        return std::string(responseData.begin(), responseData.end());
    }

private:
    std::string host_;
    int port_;
};

int main() {
    try {
        SimpleHttpClient httpClient("example.com");
        std::string response = httpClient.get("/");
        std::cout << "Response:\n" << response << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Best Practices and Tips

1. **Error Handling**: Always check the return values of methods like `connect()` and `send()`, and use `getErrorMessage()` to get detailed error information.

2. **Resource Management**: Use RAII principles to ensure proper cleanup. The `TcpClient` destructor will handle cleanup, but make sure to call `disconnect()` explicitly if you're reusing the client.

3. **Timeout Handling**: Use the timeout parameters in `connect()` and `receive()` to prevent hanging in case of network issues.

4. **Asynchronous Operations**: Utilize the asynchronous nature of `receive()` by using `std::future` to perform other tasks while waiting for data.

5. **Buffer Management**: Choose appropriate buffer sizes for `startReceiving()` and `receive()` based on your application's needs and memory constraints.

6. **Thread Safety**: The `TcpClient` class is not guaranteed to be thread-safe. If you need to access it from multiple threads, implement proper synchronization.

7. **Callback Handling**: Set up callbacks before calling `connect()` to ensure you don't miss any events.

8. **Connection State**: Always check `isConnected()` before attempting to send data or start receiving.

9. **Error Callbacks**: Implement the error callback to handle and log any errors that occur during operation.

10. **Graceful Shutdown**: Call `stopReceiving()` before `disconnect()` to ensure a clean shutdown of the receiving thread.

By following these best practices and utilizing the examples provided, you can effectively use the `TcpClient` class to create robust TCP client applications in various scenarios, from simple echo clients to more complex network protocols like HTTP.
