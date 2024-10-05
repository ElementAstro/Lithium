# FifoClient Class Documentation

The `FifoClient` class is part of the `atom::connection` namespace and provides an interface for interacting with FIFO (First In, First Out) pipes. This class allows reading from and writing to a FIFO pipe with optional timeout functionality.

## Table of Contents

1. [Class Overview](#class-overview)
2. [Constructor](#constructor)
3. [Destructor](#destructor)
4. [Public Methods](#public-methods)
   - [write](#write)
   - [read](#read)
   - [isOpen](#isopen)
   - [close](#close)
5. [Usage Examples](#usage-examples)

## Class Overview

```cpp
namespace atom::connection {

class FifoClient {
public:
    explicit FifoClient(std::string fifoPath);
    ~FifoClient();

    auto write(std::string_view data, std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> bool;
    auto read(std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> std::optional<std::string>;
    [[nodiscard]] auto isOpen() const -> bool;
    void close();

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace atom::connection
```

## Constructor

```cpp
explicit FifoClient(std::string fifoPath);
```

Creates a new `FifoClient` instance.

- **Parameters:**

  - `fifoPath`: A string representing the path to the FIFO file.

- **Description:** This constructor opens the FIFO at the specified path and prepares the client for reading and writing operations.

## Destructor

```cpp
~FifoClient();
```

Destroys the `FifoClient` instance.

- **Description:** Ensures that all resources are properly released and the FIFO is closed to prevent resource leaks.

## Public Methods

### write

```cpp
auto write(std::string_view data, std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> bool;
```

Writes data to the FIFO.

- **Parameters:**

  - `data`: The data to be written to the FIFO (as a string view).
  - `timeout`: An optional timeout for the write operation in milliseconds.

- **Returns:** `true` if the data was successfully written, `false` otherwise.

- **Description:** Attempts to write the specified data to the FIFO. If a timeout is provided, the operation will fail if it cannot complete within the given duration.

### read

```cpp
auto read(std::optional<std::chrono::milliseconds> timeout = std::nullopt) -> std::optional<std::string>;
```

Reads data from the FIFO.

- **Parameters:**

  - `timeout`: An optional timeout for the read operation in milliseconds.

- **Returns:** An `optional<string>` containing the data read from the FIFO. Returns `std::nullopt` if there's an error or no data is available.

- **Description:** Reads data from the FIFO. If a timeout is specified, it will return `std::nullopt` if the operation cannot complete within the specified time.

### isOpen

```cpp
[[nodiscard]] auto isOpen() const -> bool;
```

Checks if the FIFO is currently open.

- **Returns:** `true` if the FIFO is open, `false` otherwise.

- **Description:** Used to determine if the FIFO client is ready for operations.

### close

```cpp
void close();
```

Closes the FIFO.

- **Description:** Closes the FIFO and releases any associated resources. It's recommended to call this method when you're done using the FIFO to ensure proper cleanup.

## Usage Examples

Here are some examples demonstrating how to use the `FifoClient` class:

### Creating a FifoClient and Writing Data

```cpp
#include "fifoclient.hpp"
#include <iostream>

int main() {
    atom::connection::FifoClient client("/tmp/myfifo");

    if (client.isOpen()) {
        std::string message = "Hello, FIFO!";
        bool success = client.write(message);

        if (success) {
            std::cout << "Message written successfully." << std::endl;
        } else {
            std::cerr << "Failed to write message." << std::endl;
        }
    } else {
        std::cerr << "Failed to open FIFO." << std::endl;
    }

    return 0;
}
```

### Reading Data with a Timeout

```cpp
#include "fifoclient.hpp"
#include <iostream>
#include <chrono>

int main() {
    atom::connection::FifoClient client("/tmp/myfifo");

    if (client.isOpen()) {
        auto timeout = std::chrono::milliseconds(5000); // 5 seconds timeout
        auto result = client.read(timeout);

        if (result) {
            std::cout << "Received: " << *result << std::endl;
        } else {
            std::cout << "No data received within timeout." << std::endl;
        }
    } else {
        std::cerr << "Failed to open FIFO." << std::endl;
    }

    return 0;
}
```

### Using FifoClient in a Loop

```cpp
#include "fifoclient.hpp"
#include <iostream>
#include <chrono>
#include <thread>

int main() {
    atom::connection::FifoClient client("/tmp/myfifo");

    if (client.isOpen()) {
        while (true) {
            auto result = client.read(std::chrono::milliseconds(1000));

            if (result) {
                std::cout << "Received: " << *result << std::endl;

                // Echo back the received message
                client.write(*result);
            } else {
                std::cout << "No data received, waiting..." << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } else {
        std::cerr << "Failed to open FIFO." << std::endl;
    }

    return 0;
}
```
