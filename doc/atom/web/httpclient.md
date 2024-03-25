# HttpClient Class Documentation

## Overview

The `HttpClient` class is used for making HTTP requests using `HttpClientImpl`. It provides methods for sending GET, POST, PUT, and DELETE requests, as well as for setting SSL configurations and scanning ports.

### Constructor

```cpp
explicit HttpClient(const std::string &host, int port, bool sslEnabled);
```

- Constructs an HttpClient object with the specified `host`, `port`, and `sslEnabled` flag.

```cpp
// Create an HttpClient object with SSL enabled
HttpClient client("example.com", 443, true);
```

---

### Method: sendGetRequest

```cpp
bool sendGetRequest(const std::string &path, const std::map<std::string, std::string> &params, nlohmann::json &response, std::string &err);
```

- Sends an HTTP GET request to the specified `path` with optional `params`, and populates `response` with the server response.

```cpp
std::map<std::string, std::string> params = {{"key1", "value1"}, {"key2", "value2"}};
nlohmann::json jsonResponse;
std::string error;

bool success = client.sendGetRequest("/endpoint", params, jsonResponse, error);
if (success) {
    // Process jsonResponse
} else {
    // Handle error
}
```

---

### Method: sendPostRequest

```cpp
bool sendPostRequest(const std::string &path, const std::map<std::string, std::string> &params, const nlohmann::json &data, nlohmann::json &response, std::string &err);
```

- Sends an HTTP POST request to the specified `path` with optional `params` and `data`, and populates `response` with the server response.

```cpp
nlohmann::json postData = {
    {"key", "value"}
};
bool success = client.sendPostRequest("/endpoint", {}, postData, jsonResponse, error);
```

---

### Method: sendPutRequest

```cpp
bool sendPutRequest(const std::string &path, const std::map<std::string, std::string> &params, const nlohmann::json &data, nlohmann::json &response, std::string &err);
```

- Sends an HTTP PUT request to the specified `path` with optional `params` and `data`, and populates `response` with the server response.

```cpp
bool success = client.sendPutRequest("/endpoint", {}, postData, jsonResponse, error);
```

---

### Method: sendDeleteRequest

```cpp
bool sendDeleteRequest(const std::string &path, const std::map<std::string, std::string> &params, nlohmann::json &response, std::string &err);
```

- Sends an HTTP DELETE request to the specified `path` with optional `params`, and populates `response` with the server response.

```cpp
bool success = client.sendDeleteRequest("/endpoint", {}, jsonResponse, error);
```

---

### Method: setSslEnabled

```cpp
void setSslEnabled(bool enabled);
```

- Sets whether SSL is enabled for the HTTP client.

```cpp
client.setSslEnabled(true);
```

---

### Method: setCaCertPath

```cpp
void setCaCertPath(const std::string &path);
```

- Sets the path to the CA certificate for SSL verification.

```cpp
client.setCaCertPath("/path/to/ca_cert.pem");
```

---

### Method: setClientCertPath

```cpp
void setClientCertPath(const std::string &path);
```

- Sets the path to the client certificate for client authentication.

```cpp
client.setClientCertPath("/path/to/client_cert.pem");
```

---

### Method: setClientKeyPath

```cpp
void setClientKeyPath(const std::string &path);
```

- Sets the path to the client key for client authentication.

```cpp
client.setClientKeyPath("/path/to/client_key.pem");
```

---

### Method: scanPort

```cpp
bool scanPort(int startPort, int endPort, std::vector<int> &openPorts);
```

- Scans the range of ports between `startPort` and `endPort` and populates `openPorts` with the list of open ports.

```cpp
std::vector<int> openPorts;
bool success = client.scanPort(80, 100, openPorts);
if (success) {
    // Process openPorts
} else {
    // Handle error
}
```

---

### Method: checkServerStatus

```cpp
bool checkServerStatus(std::string &status);
```

- Checks the status of the server and populates `status` with the server status.

```cpp
std::string serverStatus;
bool success = client.checkServerStatus(serverStatus);
if (success) {
    // Process serverStatus
} else {
    // Handle error
}
```

---

### Complete Example

Here's a complete example of using the HttpClient class:

```cpp
int main() {
    HttpClient client("example.com", 443, true);

    nlohmann::json jsonResponse;
    std::string error;

    // Send a GET request
    bool successGet = client.sendGetRequest("/endpoint", {}, jsonResponse, error);

    // Send a POST request
    nlohmann::json postData = {{"key", "value"}};
    bool successPost = client.sendPostRequest("/endpoint", {}, postData, jsonResponse, error);

    // Set SSL configuration
    client.setCaCertPath("/path/to/ca_cert.pem");
    client.setClientCertPath("/path/to/client_cert.pem");
    client.setClientKeyPath("/path/to/client_key.pem");

    // Scan ports
    std::vector<int> openPorts;
    bool successScan = client.scanPort(80, 100, openPorts);

    // Check server status
    std::string serverStatus;
    bool successStatus = client.checkServerStatus(serverStatus);

    return 0;
}
```

This concludes the documentation for the HttpClient class.
