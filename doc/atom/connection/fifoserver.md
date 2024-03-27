# FifoServer Class

The `FifoServer` class provides functionality to start a server that listens on a FIFO pipe, receive messages from the pipe, and stop the server.

## Usage Example

```cpp
// Create a FifoServer object with a specified FIFO path
FifoServer server("/path/to/fifo");

// Start the FIFO server to listen for incoming messages
server.start();

// Receive a message from the FIFO pipe
std::string message = server.receiveMessage();
std::cout << "Received message: " << message << std::endl;

// Stop the FIFO server
server.stop();
```

## Methods

### `FifoServer`

Constructor for initializing the FifoServer with the FIFO pipe path.

### `start`

Starts the FIFO server to listen for incoming messages.

### `receiveMessage`

Receives a message from the FIFO pipe and returns it as a string.

### `stop`

Stops the FIFO server.

## Private Members

- `fifoPath`: The path to the FIFO pipe.
- `bufferSize`: The size of the buffer for receiving messages.
- `pipeHandle` (Windows) / `pipeFd` (Unix/Linux): Handle or file descriptor for the FIFO pipe.
