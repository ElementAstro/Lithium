# HttpClient Class Documentation

The `HttpClient` class is used to establish a connection to a server and send requests while receiving responses.

```cpp
HttpClient();
```

- Constructs an HttpClient object.

```cpp
HttpClient client;
bool success = client.initialize();
if (success) {
    // Initialization successful
} else {
    // Initialization failed
}
```

---

## Method: setErrorHandler

```cpp
void setErrorHandler(std::function<void(const std::string &)> errorHandler);
```

- Sets the error handling function for the HttpClient.

```cpp
client.setErrorHandler([](const std::string &error) {
    // Handle error, e.g., logging or displaying an error message
});
```

---

## Method: connectToServer

```cpp
bool connectToServer(const std::string &host, int port, bool useHttps);
```

- Connects to the server with the specified `host`, `port`, and whether to use HTTPS.

```cpp
bool connected = client.connectToServer("example.com", 80, false);
if (connected) {
    // Connected successfully
} else {
    // Connection failed
}
```

---

## Method: sendRequest

```cpp
bool sendRequest(const std::string &request);
```

- Sends a request to the server with the specified `request` content.

```cpp
bool requestSent = client.sendRequest("GET / HTTP/1.1\r\nHost: example.com\r\n\r\n");
if (requestSent) {
    // Request sent successfully
} else {
    // Failed to send request
}
```

---

## Method: receiveResponse

```cpp
HttpResponse receiveResponse();
```

- Receives the server's response.

```cpp
HttpResponse response = client.receiveResponse();
// Process response
```

---

## HttpRequestBuilder Class Documentation

The `HttpRequestBuilder` class is used to construct HTTP requests.

```cpp
HttpRequestBuilder(HttpMethod method, const std::string &url);
```

- Constructs an HttpRequestBuilder object with the specified `method` and `url`.

```cpp
HttpRequestBuilder builder(HttpMethod::GET, "http://example.com/api/resource");
```

---

## Method: setBody

```cpp
HttpRequestBuilder &setBody(const std::string &bodyText);
```

- Sets the body content of the request.

```cpp
builder.setBody("Some request body content");
```

---

## Method: setContentType

```cpp
HttpRequestBuilder &setContentType(const std::string &contentTypeValue);
```

- Sets the content type of the request.

```cpp
builder.setContentType("application/json");
```

---

## Method: setTimeout

```cpp
HttpRequestBuilder &setTimeout(std::chrono::seconds timeoutValue);
```

- Sets the timeout for the request.

```cpp
builder.setTimeout(std::chrono::seconds(30));
```

---

## Method: addHeader

```cpp
HttpRequestBuilder &addHeader(const std::string &key, const std::string &value);
```

- Adds a header to the request.

```cpp
builder.addHeader("Authorization", "Bearer token123");
```

---

## Method: send

```cpp
HttpResponse send();
```

- Sends the constructed HTTP request and receives the response.

```cpp
HttpResponse response = builder.send();
// Process response
```

---

## Complete Example

Here's a complete example of using the HttpClient and HttpRequestBuilder classes:

```cpp
int main() {
    HttpClient client;
    client.setErrorHandler([](const std::string &error) {
        // Handle error
    });

    bool connected = client.connectToServer("example.com", 80, false);
    if (connected) {
        HttpRequestBuilder builder(HttpMethod::GET, "/api/resource");
        HttpResponse response = builder.send();
        // Process response
    } else {
        // Connection failed
    }

    return 0;
}
```
