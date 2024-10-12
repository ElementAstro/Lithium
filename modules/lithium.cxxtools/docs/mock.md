# MockServer Documentation

## Table of Contents

1. [Overview](#overview)
2. [Dependencies](#dependencies)
3. [Class Structure](#class-structure)
4. [Configuration](#configuration)
5. [Usage](#usage)
6. [Key Features](#key-features)
7. [Examples](#examples)
8. [Error Handling](#error-handling)
9. [Logging](#logging)
10. [Security Considerations](#security-considerations)

## Overview

The MockServer is a C++ implementation of a configurable HTTPS server designed for testing and development purposes. It allows users to define custom endpoints with specific responses, delays, and headers. The server also supports serving static files and uses SSL/TLS for secure connections.

## Dependencies

The MockServer relies on the following libraries and components:

- Asio: For asynchronous I/O operations
- OpenSSL: For SSL/TLS support
- nlohmann/json: For JSON parsing
- C++17 standard library features (e.g., `<filesystem>`)

Ensure these dependencies are installed and properly linked before compiling the MockServer.

## Class Structure

The main class `MockServer` is defined as follows:

```cpp
class MockServer : public std::enable_shared_from_this<MockServer> {
public:
    MockServer(asio::io_context& ioContext, short port, const std::string& configFile);

private:
    // ... (private members and methods)
};
```

The class uses the `enable_shared_from_this` pattern to safely create `shared_ptr` instances of itself.

## Configuration

The MockServer is configured using a JSON file. The configuration file should have the following structure:

```json
{
  "endpoints": [
    {
      "path": "/example",
      "request_method": "GET",
      "response_code": 200,
      "response_body": "Hello, World!",
      "response_delay_ms": 100,
      "headers": {
        "Content-Type": "text/plain"
      }
    }
  ]
}
```

Each endpoint in the configuration file specifies:

- `path`: The URL path for the endpoint
- `request_method`: The HTTP method (e.g., GET, POST)
- `response_code`: The HTTP response code
- `response_body`: The content to be returned
- `response_delay_ms`: An optional delay before sending the response (in milliseconds)
- `headers`: An optional object containing custom headers

## Usage

To run the MockServer, compile the code and execute it with the following command-line arguments:

```
./mock_server <port> <config_file>
```

For example:

```
./mock_server 8443 config.json
```

This will start the server on port 8443 using the configuration specified in `config.json`.

## Key Features

1. **HTTPS Support**: The server uses SSL/TLS for secure connections.
2. **Configurable Endpoints**: Define custom endpoints with specific responses, status codes, and headers.
3. **Response Delay**: Simulate network latency by adding configurable delays to responses.
4. **Static File Serving**: Serve static files from a designated directory.
5. **Request Logging**: Log incoming requests to both console and file.
6. **Asynchronous Operations**: Utilizes Asio for non-blocking I/O operations.

## Examples

### 1. Defining a Simple Endpoint

In your configuration file:

```json
{
  "endpoints": [
    {
      "path": "/hello",
      "request_method": "GET",
      "response_code": 200,
      "response_body": "Hello, World!",
      "headers": {
        "Content-Type": "text/plain"
      }
    }
  ]
}
```

This creates an endpoint at `/hello` that responds with "Hello, World!" when accessed via a GET request.

### 2. Serving Static Files

Place your static files in a directory named `static` in the same location as your executable. The server will automatically serve these files when their paths are requested.

For example, if you have a file `static/example.txt`, it will be served when accessing `https://your-server:port/example.txt`.

## Error Handling

The MockServer includes basic error handling:

- If an endpoint is not found, it returns a 404 Not Found response.
- If there's an error reading a static file, it returns a 500 Internal Server Error.
- SSL/TLS errors are handled gracefully, ensuring the server continues to run.

## Logging

The MockServer logs all incoming requests:

1. To the console (stdout)
2. To a file named `request_log.txt`

The log includes the HTTP method and requested path for each incoming request.

## Security Considerations

1. **SSL/TLS Configuration**: The server uses a self-signed certificate for HTTPS. In a production environment, replace these with properly signed certificates.

2. **Input Validation**: The current implementation does not include extensive input validation. In a production setting, add robust input validation to prevent potential security vulnerabilities.

3. **Access Control**: There's no built-in authentication or authorization. Implement appropriate access control mechanisms if needed.

4. **Rate Limiting**: Consider implementing rate limiting to prevent abuse of the mock server.

Remember, this MockServer is primarily designed for testing and development purposes. Additional security measures should be implemented for any production use.

```json
{
  "endpoints": [
    {
      "path": "/api/hello",
      "request_method": "GET",
      "response_code": 200,
      "response_body": "{\"message\": \"Hello, World!\"}",
      "response_delay_ms": 500,
      "headers": {
        "Content-Type": "application/json"
      }
    },
    {
      "path": "/api/data",
      "request_method": "POST",
      "response_code": 201,
      "response_body": "{\"status\": \"Data received\"}",
      "headers": {
        "Content-Type": "application/json"
      }
    },
    {
      "path": "/api/auth",
      "request_method": "GET",
      "response_code": 401,
      "response_body": "{\"error\": \"Unauthorized\"}",
      "headers": {
        "Content-Type": "application/json",
        "WWW-Authenticate": "Basic realm=\"User Visible Realm\""
      }
    },
    {
      "path": "/slow/resource",
      "request_method": "GET",
      "response_code": 200,
      "response_body": "{\"data\": \"This response is delayed\"}",
      "response_delay_ms": 2000,
      "headers": {
        "Content-Type": "application/json"
      }
    },
    {
      "path": "/api/rate-limited",
      "request_method": "GET",
      "response_code": 429,
      "response_body": "{\"error\": \"Too Many Requests\"}",
      "headers": {
        "Content-Type": "application/json",
        "Retry-After": "30"
      }
    }
  ]
}
```
