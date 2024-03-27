# FifoClient Class

The `FifoClient` class provides functionality to connect to a FIFO pipe, send messages through the pipe, and disconnect from the pipe.

## Usage Example

```cpp
// Create a FifoClient object with a specified FIFO path
FifoClient client("/path/to/fifo");

// Connect to the FIFO pipe
client.connect();

// Send a message through the FIFO pipe
client.sendMessage("Hello, FIFO!");

// Disconnect from the FIFO pipe
client.disconnect();
```

## Methods

### `FifoClient`

Constructor for initializing the FifoClient with the FIFO pipe path.

### `connect`

Connects to the FIFO pipe.

### `sendMessage`

Sends a message through the FIFO pipe.

### `disconnect`

Disconnects from the FIFO pipe.

## Private Members

- `fifoPath`: The path to the FIFO pipe.
- `pipeHandle` (Windows) / `pipeFd` (Unix/Linux): Handle or file descriptor for the FIFO pipe.
