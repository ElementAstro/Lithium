# UDP Client Class Documentation

## Overview

The `UdpClient` class is part of the `atom::connection` namespace and provides functionality for UDP (User Datagram Protocol) communication. This class allows you to create a UDP client that can send and receive datagrams over a network.

## Class Declaration

```cpp
namespace atom::connection {
    class UdpClient {
        // ... (class members and methods)
    };
}
```

## Constructor and Destructor

### Constructor

```cpp
UdpClient();
```

Creates a new instance of the `UdpClient` class.

### Destructor

```cpp
~UdpClient();
```

Destroys the `UdpClient` instance and releases any resources.

## Public Methods

### bind

```cpp
bool bind(int port);
```

Binds the client to a specific port for receiving data.

- **Parameters:**
  - `port`: The port number to bind to.
- **Returns:** `true` if the binding is successful, `false` otherwise.

#### Example

```cpp
UdpClient client;
if (client.bind(8080)) {
    std::cout << "Client bound to port 8080" << std::endl;
} else {
    std::cerr << "Failed to bind client" << std::endl;
}
```

### send

```cpp
bool send(const std::string& host, int port, const std::vector<uint8_t>& data);
```

Sends data to a specified host and port.

- **Parameters:**
  - `host`: The destination host address.
  - `port`: The destination port number.
  - `data`: The data to be sent.
- **Returns:** `true` if the data is sent successfully, `false` otherwise.

#### Example

```cpp
std::vector<uint8_t> data = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello" in ASCII
if (client.send("192.168.1.100", 8888, data)) {
    std::cout << "Data sent successfully" << std::endl;
} else {
    std::cerr << "Failed to send data" << std::endl;
}
```

### receive

```cpp
std::vector<uint8_t> receive(
    size_t size,
    std::string& remoteHost,
    int& remotePort,
    std::chrono::milliseconds timeout = std::chrono::milliseconds::zero()
);
```

Receives data from a remote host.

- **Parameters:**
  - `size`: The number of bytes to receive.
  - `remoteHost`: Reference to store the hostname or IP address of the remote host.
  - `remotePort`: Reference to store the port number of the remote host.
  - `timeout`: The receive timeout duration (default is zero, meaning no timeout).
- **Returns:** The received data as a vector of bytes.

#### Example

```cpp
std::string remoteHost;
int remotePort;
std::vector<uint8_t> receivedData = client.receive(1024, remoteHost, remotePort, std::chrono::milliseconds(5000));
if (!receivedData.empty()) {
    std::cout << "Received " << receivedData.size() << " bytes from " << remoteHost << ":" << remotePort << std::endl;
} else {
    std::cout << "No data received within timeout" << std::endl;
}
```

### setOnDataReceivedCallback

```cpp
void setOnDataReceivedCallback(const OnDataReceivedCallback& callback);
```

Sets the callback function to be called when data is received.

- **Parameters:**
  - `callback`: The callback function with signature `void(const std::vector<uint8_t>&, const std::string&, int)`.

#### Example

```cpp
client.setOnDataReceivedCallback([](const std::vector<uint8_t>& data, const std::string& host, int port) {
    std::cout << "Received " << data.size() << " bytes from " << host << ":" << port << std::endl;
});
```

### setOnErrorCallback

```cpp
void setOnErrorCallback(const OnErrorCallback& callback);
```

Sets the callback function to be called when an error occurs.

- **Parameters:**
  - `callback`: The callback function with signature `void(const std::string&)`.

#### Example

```cpp
client.setOnErrorCallback([](const std::string& errorMessage) {
    std::cerr << "Error: " << errorMessage << std::endl;
});
```

### startReceiving

```cpp
void startReceiving(size_t bufferSize);
```

Starts receiving data asynchronously.

- **Parameters:**
  - `bufferSize`: The size of the receive buffer.

#### Example

```cpp
client.startReceiving(1024);
```

### stopReceiving

```cpp
void stopReceiving();
```

Stops receiving data.

#### Example

```cpp
client.stopReceiving();
```

## Usage Example

Here's a complete example demonstrating how to use the `UdpClient` class:

```cpp
#include "udpclient.hpp"
#include <iostream>
#include <thread>

int main() {
    atom::connection::UdpClient client;

    // Bind to a local port
    if (!client.bind(8080)) {
        std::cerr << "Failed to bind to port 8080" << std::endl;
        return 1;
    }

    // Set up callbacks
    client.setOnDataReceivedCallback([](const std::vector<uint8_t>& data, const std::string& host, int port) {
        std::cout << "Received " << data.size() << " bytes from " << host << ":" << port << std::endl;
    });

    client.setOnErrorCallback([](const std::string& errorMessage) {
        std::cerr << "Error: " << errorMessage << std::endl;
    });

    // Start receiving data
    client.startReceiving(1024);

    // Send some data
    std::vector<uint8_t> dataToSend = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello" in ASCII
    if (client.send("127.0.0.1", 8888, dataToSend)) {
        std::cout << "Data sent successfully" << std::endl;
    } else {
        std::cerr << "Failed to send data" << std::endl;
    }

    // Keep the program running for a while
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // Stop receiving
    client.stopReceiving();

    return 0;
}
```
