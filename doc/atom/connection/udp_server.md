# UdpSocketHub Class

## Description

The `UdpSocketHub` class is a simple UDP socket server class that handles incoming messages and allows sending messages to specified addresses.

## Constructor

### `UdpSocketHub()`

- Initializes the server state.

#### Example

```cpp
UdpSocketHub udpServer;
```

## Destructor

### `~UdpSocketHub()`

- Ensures proper resource cleanup.

## Methods

### `start(int port)`

- Starts the UDP server on the specified port.

#### Parameters

- `port`: The port number on which the server will listen for incoming messages.

#### Example

```cpp
udpServer.start(8888);
```

### `stop()`

- Stops the server and cleans up resources.

#### Example

```cpp
udpServer.stop();
```

### `addHandler(std::function<void(std::string)> handler)`

- Adds a message handler function that will be called whenever a new message is received.

#### Parameters

- `handler`: A function to handle incoming messages. It takes a string as input.

#### Example

```cpp
udpServer.addHandler([](std::string message) {
    std::cout << "Received message: " << message << std::endl;
});
```

### `sendTo(const std::string &message, const std::string &ip, int port)`

- Sends a message to the specified IP address and port.

#### Parameters

- `message`: The message to be sent.
- `ip`: The target IP address.
- `port`: The target port number.

#### Example

```cpp
udpServer.sendTo("Hello, UDP!", "192.168.1.100", 9999);
```

## Private Members

- `m_running`: Indicates whether the server is running.
- `m_serverSocket`: The socket descriptor for the server.
- `m_acceptThread`: The thread for handling incoming messages.
- `m_handler`: The function to handle incoming messages.

### Private Methods

#### `initNetworking()`

- Initializes networking. Required for Windows.
- Returns true if successful, false otherwise.

#### `cleanupNetworking()`

- Cleans up networking resources. Required for Windows.

#### `handleMessages()`

- The main loop for receiving messages. Runs in a separate thread.
