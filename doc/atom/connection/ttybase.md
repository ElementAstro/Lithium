# TTYBase Class Documentation

The `TTYBase` class provides a foundation for handling TTY (Teletypewriter) connections. It offers methods for reading from and writing to TTY devices, as well as managing connections and error handling.

## Table of Contents

1. [Class Overview](#class-overview)
2. [Enumerations](#enumerations)
3. [Constructor and Destructor](#constructor-and-destructor)
4. [Public Methods](#public-methods)
   - [read](#read)
   - [readSection](#readsection)
   - [write](#write)
   - [writeString](#writestring)
   - [connect](#connect)
   - [disconnect](#disconnect)
   - [setDebug](#setdebug)
   - [getErrorMessage](#geterrormessage)
   - [getPortFD](#getportfd)
5. [Usage Examples](#usage-examples)
6. [Best Practices](#best-practices)

## Class Overview

```cpp
class TTYBase {
public:
    enum class TTYResponse { ... };

    explicit TTYBase(std::string_view driverName);
    virtual ~TTYBase();

    // Public methods
    // ...

private:
    TTYResponse checkTimeout(uint8_t timeout);

    int m_PortFD{-1};
    bool m_Debug{false};
    std::string_view m_DriverName;
};
```

## Enumerations

### TTYResponse

```cpp
enum class TTYResponse {
    OK = 0,
    ReadError = -1,
    WriteError = -2,
    SelectError = -3,
    Timeout = -4,
    PortFailure = -5,
    ParamError = -6,
    Errno = -7,
    Overflow = -8
};
```

This enumeration represents possible responses from TTY operations.

## Constructor and Destructor

### Constructor

```cpp
explicit TTYBase(std::string_view driverName);
```

Constructs a TTYBase instance with the specified driver name.

- **Parameters:**
  - `driverName`: The name of the TTY driver to be used.

### Destructor

```cpp
virtual ~TTYBase();
```

Cleans up resources associated with the TTY connection.

## Public Methods

### read

```cpp
TTYResponse read(uint8_t* buffer, uint32_t nbytes, uint8_t timeout, uint32_t& nbytesRead);
```

Reads data from the TTY device.

- **Parameters:**
  - `buffer`: Pointer to the buffer where read data will be stored.
  - `nbytes`: The number of bytes to read from the TTY.
  - `timeout`: Timeout duration for the read operation in seconds.
  - `nbytesRead`: Reference to store the actual number of bytes read.
- **Returns:** `TTYResponse` indicating the result of the read operation.

### readSection

```cpp
TTYResponse readSection(uint8_t* buffer, uint32_t nsize, uint8_t stopByte, uint8_t timeout, uint32_t& nbytesRead);
```

Reads a section of data from the TTY until a stop byte is encountered.

- **Parameters:**
  - `buffer`: Pointer to the buffer where read data will be stored.
  - `nsize`: The maximum number of bytes to read.
  - `stopByte`: The byte value that will stop the reading.
  - `timeout`: Timeout duration for the read operation in seconds.
  - `nbytesRead`: Reference to store the actual number of bytes read.
- **Returns:** `TTYResponse` indicating the result of the read operation.

### write

```cpp
TTYResponse write(const uint8_t* buffer, uint32_t nbytes, uint32_t& nbytesWritten);
```

Writes data to the TTY device.

- **Parameters:**
  - `buffer`: Pointer to the data to be written.
  - `nbytes`: The number of bytes to write to the TTY.
  - `nbytesWritten`: Reference to store the actual number of bytes written.
- **Returns:** `TTYResponse` indicating the result of the write operation.

### writeString

```cpp
TTYResponse writeString(std::string_view string, uint32_t& nbytesWritten);
```

Writes a string to the TTY device.

- **Parameters:**
  - `string`: The string to be written to the TTY.
  - `nbytesWritten`: Reference to store the actual number of bytes written.
- **Returns:** `TTYResponse` indicating the result of the write operation.

### connect

```cpp
TTYResponse connect(std::string_view device, uint32_t bitRate, uint8_t wordSize, uint8_t parity, uint8_t stopBits);
```

Connects to the specified TTY device.

- **Parameters:**
  - `device`: The device name or path to connect to.
  - `bitRate`: The baud rate for the connection.
  - `wordSize`: The data size (in bits) of each character.
  - `parity`: The parity checking mode (e.g., none, odd, even).
  - `stopBits`: The number of stop bits to use in communication.
- **Returns:** `TTYResponse` indicating the result of the connection attempt.

### disconnect

```cpp
TTYResponse disconnect();
```

Disconnects from the TTY device.

- **Returns:** `TTYResponse` indicating the result of the disconnection.

### setDebug

```cpp
void setDebug(bool enabled);
```

Enables or disables debugging information.

- **Parameters:**
  - `enabled`: `true` to enable debugging, `false` to disable it.

### getErrorMessage

```cpp
std::string getErrorMessage(TTYResponse code) const;
```

Retrieves an error message corresponding to a given TTYResponse code.

- **Parameters:**
  - `code`: The TTYResponse code for which to get the error message.
- **Returns:** A string containing the error message.

### getPortFD

```cpp
int getPortFD() const;
```

Gets the file descriptor for the TTY port.

- **Returns:** The integer file descriptor for the TTY port.

## Usage Examples

Here are some examples demonstrating how to use the `TTYBase` class:

### Connecting to a TTY Device

```cpp
#include "ttybase.hpp"
#include <iostream>

int main() {
    TTYBase tty("MyDriver");

    TTYBase::TTYResponse response = tty.connect("/dev/ttyUSB0", 9600, 8, 0, 1);
    if (response == TTYBase::TTYResponse::OK) {
        std::cout << "Successfully connected to TTY device." << std::endl;
    } else {
        std::cerr << "Failed to connect: " << tty.getErrorMessage(response) << std::endl;
        return 1;
    }

    // Use the TTY connection...

    tty.disconnect();
    return 0;
}
```

### Reading from a TTY Device

```cpp
TTYBase tty("MyDriver");
// Assume we've already connected to the device

uint8_t buffer[256];
uint32_t bytesRead;
TTYBase::TTYResponse response = tty.read(buffer, sizeof(buffer), 5, bytesRead);

if (response == TTYBase::TTYResponse::OK) {
    std::cout << "Read " << bytesRead << " bytes: ";
    for (uint32_t i = 0; i < bytesRead; ++i) {
        std::cout << std::hex << static_cast<int>(buffer[i]) << " ";
    }
    std::cout << std::endl;
} else {
    std::cerr << "Failed to read: " << tty.getErrorMessage(response) << std::endl;
}
```

### Writing to a TTY Device

```cpp
#include "ttybase.hpp"
#include <iostream>
#include <thread>
#include <atomic>

class SimpleTerminal {
public:
    SimpleTerminal(const std::string& device) : m_tty("SimpleTerminal") {
        if (m_tty.connect(device, 9600, 8, 0, 1) != TTYBase::TTYResponse::OK) {
            throw std::runtime_error("Failed to connect to TTY device");
        }
    }

    ~SimpleTerminal() {
        m_running = false;
        if (m_readThread.joinable()) {
            m_readThread.join();
        }
        m_tty.disconnect();
    }

    void run() {
        m_readThread = std::thread(&SimpleTerminal::readLoop, this);
        writeLoop();
    }

private:
    void readLoop() {
        uint8_t buffer[256];
        uint32_t bytesRead;

        while (m_running) {
            TTYBase::TTYResponse response = m_tty.read(buffer, sizeof(buffer), 1, bytesRead);
            if (response == TTYBase::TTYResponse::OK && bytesRead > 0) {
                std::cout.write(reinterpret_cast<char*>(buffer), bytesRead);
                std::cout.flush();
            }
        }
    }

    void writeLoop() {
        std::string input;
        while (m_running && std::getline(std::cin, input)) {
            input += '\n';  // Add newline
            uint32_t bytesWritten;
            TTYBase::TTYResponse response = m_tty.writeString(input, bytesWritten);
            if (response != TTYBase::TTYResponse::OK) {
                std::cerr << "Write failed: " << m_tty.getErrorMessage(response) << std::endl;
            }
        }
    }

    TTYBase m_tty;
    std::thread m_readThread;
    std::atomic<bool> m_running{true};
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <tty_device>" << std::endl;
        return 1;
    }

    try {
        SimpleTerminal terminal(argv[1]);
        terminal.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
```

This example creates a simple terminal emulator that continuously reads from the TTY device and writes user input to it.

### Implementing a Protocol Parser

If you're working with a specific protocol over the TTY, you might want to implement a parser. Here's a basic example for a hypothetical protocol:

```cpp
#include "ttybase.hpp"
#include <vector>
#include <iostream>

class ProtocolParser {
public:
    ProtocolParser(TTYBase& tty) : m_tty(tty) {}

    bool readMessage(std::vector<uint8_t>& message) {
        uint8_t header[2];
        uint32_t bytesRead;

        // Read header
        if (m_tty.read(header, 2, 5, bytesRead) != TTYBase::TTYResponse::OK || bytesRead != 2) {
            return false;
        }

        // Check start byte
        if (header[0] != 0xAA) {
            return false;
        }

        uint8_t length = header[1];
        message.resize(length);

        // Read message body
        if (m_tty.read(message.data(), length, 5, bytesRead) != TTYBase::TTYResponse::OK || bytesRead != length) {
            return false;
        }

        return true;
    }

    bool sendMessage(const std::vector<uint8_t>& message) {
        if (message.size() > 255) {
            return false;
        }

        std::vector<uint8_t> fullMessage;
        fullMessage.push_back(0xAA);  // Start byte
        fullMessage.push_back(static_cast<uint8_t>(message.size()));
        fullMessage.insert(fullMessage.end(), message.begin(), message.end());

        uint32_t bytesWritten;
        return m_tty.write(fullMessage.data(), fullMessage.size(), bytesWritten) == TTYBase::TTYResponse::OK
            && bytesWritten == fullMessage.size();
    }

private:
    TTYBase& m_tty;
};

// Usage example
int main() {
    TTYBase tty("ProtocolDevice");
    if (tty.connect("/dev/ttyUSB0", 115200, 8, 0, 1) != TTYBase::TTYResponse::OK) {
        std::cerr << "Failed to connect to TTY device" << std::endl;
        return 1;
    }

    ProtocolParser parser(tty);

    // Send a message
    std::vector<uint8_t> messageToSend = {0x01, 0x02, 0x03, 0x04};
    if (parser.sendMessage(messageToSend)) {
        std::cout << "Message sent successfully" << std::endl;
    } else {
        std::cerr << "Failed to send message" << std::endl;
    }

    // Read a message
    std::vector<uint8_t> receivedMessage;
    if (parser.readMessage(receivedMessage)) {
        std::cout << "Received message: ";
        for (uint8_t byte : receivedMessage) {
            std::cout << std::hex << static_cast<int>(byte) << " ";
        }
        std::cout << std::endl;
    } else {
        std::cerr << "Failed to read message" << std::endl;
    }

    tty.disconnect();
    return 0;
}
```

## Performance Considerations

When working with TTY devices, especially in high-throughput scenarios, consider the following performance tips:

1. **Buffer Sizes**: Choose appropriate buffer sizes for your read and write operations. Larger buffers can improve throughput but may increase latency.

2. **Timeout Values**: Adjust timeout values based on your application's needs. Shorter timeouts can make your application more responsive, but may result in more partial reads.

3. **Polling vs. Event-Driven**: For high-performance applications, consider using an event-driven approach instead of continuous polling.

4. **Baud Rate**: Use the highest baud rate that your device and application can reliably handle.

5. **Batching**: When possible, batch multiple small writes into larger ones to reduce system call overhead.
