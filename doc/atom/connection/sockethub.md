# SocketHub Class Documentation

The `SocketHub` class is designed to manage socket connections and provides functionalities for starting and stopping a socket service while managing multiple client connections.

## Constructor

### SocketHub()

This is the constructor for the `SocketHub` class.

#### Example

```cpp
SocketHub socket;  // Create an instance of the SocketHub class
```

## Destructor

### ~SocketHub()

The destructor is responsible for cleaning up resources associated with the `SocketHub` class instance.

#### Example

```cpp
SocketHub* socket = new SocketHub();
delete socket;  // Cleanup resources when the object is no longer needed
```

## Public Methods

### void start(int port)

Starts the socket service and listens on the specified port.

#### Parameters

- `port`: The port number to listen on.

#### Example

```cpp
SocketHub socket;
socket.start(8080);  // Start the socket service on port 8080
```

### void stop()

Stops the socket service and closes all connections.

#### Example

```cpp
socket.stop();  // Stop the socket service and close all connections
```

### void addHandler(std::function<void(std::string)> handler)

Adds a message handler to the `SocketHub`.

#### Parameters

- `handler`: The message handler function.

#### Example

```cpp
void handleMessage(std::string message) {
    // Handle the incoming message
    std::cout << "Received message: " << message << std::endl;
}

SocketHub socket;
socket.addHandler(handleMessage);  // Add the message handling function
```

## Private Methods

### bool initWinsock()

Initializes Winsock.

#### Returns

- `true` on success, `false` on failure.

### void cleanupWinsock()

Cleans up Winsock resources.

### void closeSocket(SOCKET socket) / void closeSocket(int socket)

Closes the specified socket.

### void acceptConnections()

Accepts client connections and adds them to the clients list.

### void handleClientMessages(SOCKET clientSocket) / void handleClientMessages(int clientSocket)

Handles messages from a client.

### void cleanupSocket()

Cleans up socket resources, closing all client connections.

These private methods are used internally by the `SocketHub` class and are not intended for direct usage by external code.

## Usage

```cpp
int main() {
    SocketHub socket;
    socket.start(8080);

    // Define a message handling function
    auto handleMessage = [](std::string message) {
        std::cout << "Received message: " << message << std::endl;
    };

    socket.addHandler(handleMessage);

    // Other operations...

    socket.stop();
    return 0;
}
```
