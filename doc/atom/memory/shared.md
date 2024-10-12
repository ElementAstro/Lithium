# SharedMemory Documentation

## Table of Contents

1. [Introduction](#introduction)
2. [Class Overview](#class-overview)
3. [Constructor and Destructor](#constructor-and-destructor)
4. [Public Methods](#public-methods)
5. [Usage Examples](#usage-examples)

## Introduction

The `SharedMemory` class is a C++ template class that provides a cross-platform implementation of shared memory for inter-process communication. It allows processes to share data of a specified type `T` through a named shared memory segment. The class ensures thread-safe access to the shared memory and provides various methods for reading and writing data.

## Class Overview

```cpp
namespace atom::connection {

template <TriviallyCopyable T>
class SharedMemory : public NonCopyable {
public:
    explicit SharedMemory(std::string_view name, bool create = true);
    ~SharedMemory() override;

    // Public methods...

private:
    // Private members and methods...
};

} // namespace atom::connection
```

The `SharedMemory` class is templated on type `T`, which must satisfy the `TriviallyCopyable` concept. This ensures that the data can be safely copied between processes.

## Constructor and Destructor

### Constructor

```cpp
explicit SharedMemory(std::string_view name, bool create = true);
```

Creates or opens a shared memory segment.

- `name`: The name of the shared memory segment.
- `create`: If `true`, creates a new segment; if `false`, opens an existing segment.

### Destructor

```cpp
~SharedMemory() override;
```

Cleans up resources associated with the shared memory segment.

## Public Methods

### write

```cpp
void write(const T& data, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
```

Writes data to the shared memory segment.

- `data`: The data to write.
- `timeout`: Maximum time to wait for the lock. Default is no timeout.

### read

```cpp
ATOM_NODISCARD auto read(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const -> T;
```

Reads data from the shared memory segment.

- `timeout`: Maximum time to wait for the lock. Default is no timeout.
- Returns: The data read from the shared memory.

### clear

```cpp
void clear();
```

Clears the contents of the shared memory segment.

### isOccupied

```cpp
ATOM_NODISCARD auto isOccupied() const -> bool;
```

Checks if the shared memory is currently locked.

### getName

```cpp
ATOM_NODISCARD auto getName() const ATOM_NOEXCEPT -> std::string_view;
```

Returns the name of the shared memory segment.

### getSize

```cpp
ATOM_NODISCARD auto getSize() const ATOM_NOEXCEPT -> std::size_t;
```

Returns the size of the shared memory segment.

### isCreator

```cpp
ATOM_NODISCARD auto isCreator() const ATOM_NOEXCEPT -> bool;
```

Returns whether this instance created the shared memory segment.

### writePartial

```cpp
template <typename U>
void writePartial(const U& data, std::size_t offset, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
```

Writes a partial amount of data to the shared memory segment.

- `data`: The data to write.
- `offset`: The offset within the shared memory to write to.
- `timeout`: Maximum time to wait for the lock. Default is no timeout.

### readPartial

```cpp
template <typename U>
ATOM_NODISCARD auto readPartial(std::size_t offset, std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const -> U;
```

Reads a partial amount of data from the shared memory segment.

- `offset`: The offset within the shared memory to read from.
- `timeout`: Maximum time to wait for the lock. Default is no timeout.
- Returns: The data read from the shared memory.

### tryRead

```cpp
ATOM_NODISCARD auto tryRead(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const -> std::optional<T>;
```

Attempts to read data from the shared memory segment.

- `timeout`: Maximum time to wait for the lock. Default is no timeout.
- Returns: An optional containing the data if successful, or `std::nullopt` if unsuccessful.

### writeSpan

```cpp
void writeSpan(std::span<const std::byte> data, std::chrono::milliseconds timeout = std::chrono::milliseconds(0));
```

Writes a span of bytes to the shared memory segment.

- `data`: The span of bytes to write.
- `timeout`: Maximum time to wait for the lock. Default is no timeout.

### readSpan

```cpp
ATOM_NODISCARD auto readSpan(std::span<std::byte> data, std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) const -> std::size_t;
```

Reads a span of bytes from the shared memory segment.

- `data`: The span to read into.
- `timeout`: Maximum time to wait for the lock. Default is no timeout.
- Returns: The number of bytes read.

## Usage Examples

### Basic Usage

```cpp
#include "shared_memory.hpp"
#include <iostream>

struct MyData {
    int value;
    char message[20];
};

int main() {
    try {
        // Create shared memory
        atom::connection::SharedMemory<MyData> shm("my_shared_memory");

        // Write data
        MyData writeData = {42, "Hello, World!"};
        shm.write(writeData);

        // Read data
        MyData readData = shm.read();

        std::cout << "Read value: " << readData.value << std::endl;
        std::cout << "Read message: " << readData.message << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Using Timeouts

```cpp
#include "shared_memory.hpp"
#include <iostream>
#include <chrono>

int main() {
    try {
        atom::connection::SharedMemory<int> shm("int_shared_memory");

        // Write with timeout
        shm.write(123, std::chrono::milliseconds(100));

        // Read with timeout
        auto result = shm.tryRead(std::chrono::milliseconds(100));
        if (result) {
            std::cout << "Read value: " << *result << std::endl;
        } else {
            std::cout << "Failed to read within timeout" << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Partial Reads and Writes

```cpp
#include "shared_memory.hpp"
#include <iostream>
#include <array>

struct ComplexData {
    int id;
    double values[5];
    char name[20];
};

int main() {
    try {
        atom::connection::SharedMemory<ComplexData> shm("complex_data");

        // Write partial data
        int newId = 42;
        shm.writePartial(newId, offsetof(ComplexData, id));

        std::array<double, 2> newValues = {3.14, 2.71};
        shm.writePartial(newValues, offsetof(ComplexData, values));

        // Read partial data
        auto readId = shm.readPartial<int>(offsetof(ComplexData, id));
        auto readName = shm.readPartial<char[20]>(offsetof(ComplexData, name));

        std::cout << "Read ID: " << readId << std::endl;
        std::cout << "Read Name: " << readName << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Using Spans

```cpp
#include "shared_memory.hpp"
#include <iostream>
#include <vector>
#include <span>

int main() {
    try {
        atom::connection::SharedMemory<std::array<std::byte, 1024>> shm("large_data");

        // Write data using span
        std::vector<std::byte> writeData(500);
        // Fill writeData with some values...
        std::span<const std::byte> writeSpan(writeData);
        shm.writeSpan(writeSpan);

        // Read data using span
        std::vector<std::byte> readData(1024);
        std::span<std::byte> readSpan(readData);
        size_t bytesRead = shm.readSpan(readSpan);

        std::cout << "Bytes read: " << bytesRead << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Inter-Process Communication

This example demonstrates how to use `SharedMemory` for communication between two processes. You'll need to run two separate programs: a writer and a reader.

Writer process:

```cpp
#include "shared_memory.hpp"
#include <iostream>
#include <thread>
#include <chrono>

struct Message {
    int id;
    char content[256];
};

int main() {
    try {
        atom::connection::SharedMemory<Message> shm("ipc_example");

        for (int i = 0; i < 5; ++i) {
            Message msg;
            msg.id = i;
            snprintf(msg.content, sizeof(msg.content), "Message %d", i);

            shm.write(msg);
            std::cout << "Wrote message: " << msg.content << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

Reader process:

```cpp
#include "shared_memory.hpp"
#include <iostream>
#include <chrono>

struct Message {
    int id;
    char content[256];
};

int main() {
    try {
        atom::connection::SharedMemory<Message> shm("ipc_example", false); // Open existing

        for (int i = 0; i < 5; ++i) {
            auto result = shm.tryRead(std::chrono::seconds(2));
            if (result) {
                std::cout << "Read message: " << result->content << std::endl;
            } else {
                std::cout << "Timeout waiting for message" << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Error Handling

The `SharedMemory` class uses exceptions to handle errors. Here's an example of how to handle various exceptions that might be thrown:

```cpp
#include "shared_memory.hpp"
#include <iostream>

int main() {
    try {
        atom::connection::SharedMemory<int> shm("error_example");

        // Try to write with a very short timeout
        try {
            shm.write(42, std::chrono::microseconds(1));
        } catch (const std::exception& e) {
            std::cerr << "Write failed: " << e.what() << std::endl;
        }

        // Try to read from an invalid offset
        try {
            shm.readPartial<double>(sizeof(int));
        } catch (const std::invalid_argument& e) {
            std::cerr << "Invalid read: " << e.what() << std::endl;
        }

        // Try to create a shared memory with an existing name (platform-specific behavior)
        try {
            atom::connection::SharedMemory<int> shm2("error_example");
        } catch (const std::exception& e) {
            std::cerr << "Failed to create shared memory: " << e.what() << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
    }

    return 0;
}
```

### Advanced Usage: Circular Buffer in Shared Memory

This example demonstrates how to implement a simple circular buffer using `SharedMemory`, which can be useful for producer-consumer scenarios across processes.

```cpp
#include "shared_memory.hpp"
#include <iostream>
#include <array>

template<typename T, size_t N>
struct CircularBuffer {
    std::array<T, N> buffer;
    size_t head = 0;
    size_t tail = 0;
    bool full = false;
};

template<typename T, size_t N>
class SharedCircularBuffer {
public:
    SharedCircularBuffer(const std::string& name) : shm_(name) {}

    void push(const T& item) {
        auto buf = shm_.read();
        if (buf.full && buf.head == buf.tail) {
            buf.tail = (buf.tail + 1) % N;
        }
        buf.buffer[buf.head] = item;
        buf.head = (buf.head + 1) % N;
        buf.full = buf.head == buf.tail;
        shm_.write(buf);
    }

    std::optional<T> pop() {
        auto buf = shm_.read();
        if (buf.head == buf.tail && !buf.full) {
            return std::nullopt;
        }
        T item = buf.buffer[buf.tail];
        buf.tail = (buf.tail + 1) % N;
        buf.full = false;
        shm_.write(buf);
        return item;
    }

private:
    atom::connection::SharedMemory<CircularBuffer<T, N>> shm_;
};

int main() {
    try {
        SharedCircularBuffer<int, 5> buffer("circular_buffer_example");

        // Producer
        for (int i = 0; i < 7; ++i) {
            buffer.push(i);
            std::cout << "Pushed: " << i << std::endl;
        }

        // Consumer
        for (int i = 0; i < 7; ++i) {
            auto item = buffer.pop();
            if (item) {
                std::cout << "Popped: " << *item << std::endl;
            } else {
                std::cout << "Buffer is empty" << std::endl;
            }
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
```

These examples demonstrate various use cases and advanced techniques for using the `SharedMemory` class. They cover inter-process communication, error handling, and implementing more complex data structures in shared memory. Remember to compile and link against the necessary libraries and include the appropriate headers when using these examples.
