# Network Client Application Documentation

## Overview

The `Network Client Application` is a tool designed to send files or messages over TCP or UDP protocols. It uses the `ASIO` library for network communication and `loguru` for logging. The application supports both TCP and UDP modes, allowing users to specify a timeout for TCP connections and optionally send a file.

## Dependencies

- **ASIO**: A cross-platform C++ library for network and low-level I/O programming.
- **loguru**: A logging library used for logging operations.

## Constants

- **MAX_LENGTH**: The maximum length of the buffer used for reading and sending data.
- **ARG_COUNT_MIN**: The minimum number of command-line arguments required.
- **ARG_COUNT_MAX**: The maximum number of command-line arguments allowed.
- **DEFAULT_TIMEOUT_SECONDS**: The default timeout in seconds for TCP connections.

## Functions

### `sendFileTcp(tcp::socket& socket, const std::string& filename)`

Sends a file over a TCP connection.

- **Parameters:**

  - `socket`: The TCP socket to send the file through.
  - `filename`: The name of the file to send.

- **Example:**
  ```cpp
  asio::io_context ioContext;
  tcp::resolver resolver(ioContext);
  auto endpoints = resolver.resolve("localhost", "12345");
  tcp::socket socket(ioContext);
  asio::connect(socket, endpoints);
  sendFileTcp(socket, "example.txt");
  ```

### `sendFileUdp(udp::socket& socket, const udp::endpoint& endpoint, const std::string& filename)`

Sends a file over a UDP connection.

- **Parameters:**

  - `socket`: The UDP socket to send the file through.
  - `endpoint`: The endpoint to send the file to.
  - `filename`: The name of the file to send.

- **Example:**
  ```cpp
  asio::io_context ioContext;
  udp::resolver resolver(ioContext);
  udp::resolver::results_type endpoints = resolver.resolve(udp::v4(), "localhost", "12345");
  udp::socket socket(ioContext);
  socket.open(udp::v4());
  sendFileUdp(socket, *endpoints.begin(), "example.txt");
  ```

### `runTcpClient(const std::string& host, const std::string& port, int timeoutSeconds, const std::optional<std::string>& filename = std::nullopt)`

Runs the TCP client mode.

- **Parameters:**

  - `host`: The host to connect to.
  - `port`: The port to connect to.
  - `timeoutSeconds`: The timeout in seconds for the TCP connection.
  - `filename`: An optional filename to send over the TCP connection.

- **Example:**
  ```cpp
  runTcpClient("localhost", "12345", 10, "example.txt");
  ```

### `runUdpClient(const std::string& host, const std::string& port, const std::optional<std::string>& filename = std::nullopt)`

Runs the UDP client mode.

- **Parameters:**

  - `host`: The host to connect to.
  - `port`: The port to connect to.
  - `filename`: An optional filename to send over the UDP connection.

- **Example:**
  ```cpp
  runUdpClient("localhost", "12345", "example.txt");
  ```

### `main(int argc, char* argv[]) -> int`

The main function that initializes the application, parses command-line arguments, and starts the appropriate client mode.

- **Parameters:**

  - `argc`: The number of command-line arguments.
  - `argv`: The array of command-line arguments.

- **Returns:** An integer representing the exit status.

- **Example:**
  ```bash
  ./nc tcp localhost 12345 10 example.txt
  ```

## Notes

- The application supports both TCP and UDP protocols.
- Users can specify a timeout for TCP connections.
- The application can optionally send a file over the network.
- Logging is used extensively to provide detailed information about the operations being performed.
