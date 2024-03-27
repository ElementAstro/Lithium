# SerialPort Class Documentation

## Overview

The `SerialPort` class provides functionality to interact with serial ports. It allows opening, closing, reading, and writing data to a serial port with specified configurations.

### Constructor

```cpp
explicit SerialPort(const std::string &portName, int baudRate, int dataBits, Parity parity, StopBits stopBits);
```

#### Usage Example

```cpp
SerialPort serial("COM1", 9600, 8, Parity::None, StopBits::One);
```

---

## open Method

Opens the serial port for communication.

```cpp
bool open();
```

#### Usage Example

```cpp
bool isOpen = serial.open();
// Expected output: true if the serial port is successfully opened
```

---

## close Method

Closes the serial port.

```cpp
void close();
```

#### Usage Example

```cpp
serial.close();
```

---

## read Method

Reads data from the serial port into a buffer.

```cpp
bool read(char *buffer, int bufferSize);
```

#### Usage Example

```cpp
char data[256];
bool success = serial.read(data, 256);
// Expected output: true if data is successfully read into the buffer
```

---

## write Method

Writes data to the serial port.

```cpp
bool write(const char *data, int dataSize);
```

#### Usage Example

```cpp
const char *message = "Hello, Serial!";
bool success = serial.write(message, strlen(message));
// Expected output: true if data is successfully written to the serial port
```

---

## getAvailablePorts Method (in SerialPortFactory)

Returns a list of available serial ports.

```cpp
static std::vector<std::string> getAvailablePorts();
```

#### Usage Example

```cpp
std::vector<std::string> ports = SerialPortFactory::getAvailablePorts();
// Expected output: A vector containing the names of available serial ports
```

---

## createSerialPort Method (in SerialPortFactory)

Creates a new `SerialPort` instance with specified configurations.

```cpp
static SerialPort createSerialPort(const std::string &portName, int baudRate, int dataBits, Parity parity, StopBits stopBits);
```

#### Usage Example

```cpp
SerialPort newSerial = SerialPortFactory::createSerialPort("COM2", 115200, 8, Parity::Even, StopBits::Two);
```

---

## Special Notes

1. For Windows platform, the `HANDLE` type is used for handling the serial port.
2. For non-Windows platforms, an integer handle is used for the serial port.
