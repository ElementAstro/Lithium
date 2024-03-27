# TTYBase Class

The `TTYBase` class provides functionality for interacting with terminal devices.

## Constructor

## `TTYBase(const char *driverName)`

Creates a `TTYBase` object with the specified driver name.

**Example:**

```cpp
TTYBase tty("ttyS0");
```

---

## Destructor

## `virtual ~TTYBase()`

Destroys the `TTYBase` object and cleans up any resources.

**Example:**

```cpp
// No direct example as this is implicitly called when object goes out of scope
```

---

## `read` Method

## `TTY_RESPONSE read(uint8_t *buffer, uint32_t nbytes, uint8_t timeout, uint32_t *nbytes_read)`

Reads data from the terminal device.

**Parameters:**

- `buffer`: Pointer to store data.
- `nbytes`: Number of bytes to read.
- `timeout`: Number of seconds to wait for terminal before timeout error.
- `nbytes_read`: Number of bytes read.

**Example:**

```cpp
uint8_t data[256];
uint32_t bytesRead;
TTYBase::TTY_RESPONSE response = tty.read(data, 128, 5, &bytesRead);
if (response == TTYBase::TTY_OK) {
    // Process the data read
} else {
    // Handle error
}
```

---

## `readSection` Method

## `TTY_RESPONSE readSection(uint8_t *buffer, uint32_t nsize, uint8_t stop_byte, uint8_t timeout, uint32_t *nbytes_read)`

Reads data with a delimiter from the terminal device.

**Parameters:**

- `buffer`: Pointer to store data.
- `nsize`: Size of buffer.
- `stop_byte`: Stop character to terminate reading.
- `timeout`: Timeout value in seconds.
- `nbytes_read`: Number of bytes read.

**Example:**

```cpp
uint8_t data[256];
uint32_t bytesRead;
TTYBase::TTY_RESPONSE response = tty.readSection(data, 256, '\n', 5, &bytesRead);
if (response == TTYBase::TTY_OK) {
    // Process the data read until newline character is encountered
} else {
    // Handle error
}
```

---

## `write` Method

## `TTY_RESPONSE write(const uint8_t *buffer, uint32_t nbytes, uint32_t *nbytes_written)`

Writes data to the terminal device.

**Parameters:**

- `buffer`: Data buffer to write.
- `nbytes`: Number of bytes to write.
- `nbytes_written`: Number of bytes actually written.

**Example:**

```cpp
uint8_t data[] = {0x01, 0x02, 0x03};
uint32_t bytesWritten;
TTYBase::TTY_RESPONSE response = tty.write(data, 3, &bytesWritten);
if (response == TTYBase::TTY_OK) {
    // Data successfully written
} else {
    // Handle error
}
```

---

## `writeString` Method

## `TTY_RESPONSE writeString(const char *string, uint32_t *nbytes_written)`

Writes a null-terminated string to the terminal device.

**Parameters:**

- `string`: Null-terminated string to write.
- `nbytes_written`: Number of bytes written.

**Example:**

```cpp
const char *message = "Hello, World!";
uint32_t bytesWritten;
TTYBase::TTY_RESPONSE response = tty.writeString(message, &bytesWritten);
if (response == TTYBase::TTY_OK) {
    // String successfully written
} else {
    // Handle error
}
```

---

## `connect` Method

## `TTY_RESPONSE connect(const char *device, uint32_t bit_rate, uint8_t word_size, uint8_t parity, uint8_t stop_bits)`

Establishes a connection to a terminal device.

**Parameters:**

- `device`: Device node (e.g., /dev/ttyS0).
- `bit_rate`: Bit rate for communication.
- `word_size`: Number of data bits (7 or 8).
- `parity`: Parity setting (0=no parity, 1=even, 2=odd).
- `stop_bits`: Number of stop bits (1 or 2).

**Example:**

```cpp
TTYBase::TTY_RESPONSE response = tty.connect("/dev/ttyS0", 9600, 8, 0, 1);
if (response == TTYBase::TTY_OK) {
    // Connection successful
} else {
    // Handle connection error
}
```

---

## `disconnect` Method

## `TTY_RESPONSE disconnect()`

Closes the tty connection and flushes the bus.

**Example:**

```cpp
TTYBase::TTY_RESPONSE response = tty.disconnect();
if (response == TTYBase::TTY_OK) {
    // Disconnected successfully
} else {
    // Handle disconnection error
}
```

---

## `setDebug` Method

## `void setDebug(HYDROGEN::Logger::VerbosityLevel channel)`

Enables or disables debug logging for TTY traffic.

**Parameters:**

- `channel`: Logging verbosity level.

**Special Note:**

- Use debug logging cautiously as it may impact driver function due to verbose traffic.

**Example:**

```cpp
tty.setDebug(HYDROGEN::Logger::DBG_INFO);
```

---

## `error` Method

## `const std::string error(TTY_RESPONSE code) const`

Retrieves the error message corresponding to the given TTY response code.

**Example:**

```cpp
TTYBase::TTY_RESPONSE response = tty.read(data, 128, 5, &bytesRead);
if (response != TTYBase::TTY_OK) {
    std::string errorMessage = tty.error(response);
    // Print or handle error message
}
```

---

## `getPortFD` Method

## `int getPortFD() const`

Returns the file descriptor of the port.

**Example:**

```cpp
int portFD = tty.getPortFD();
```
