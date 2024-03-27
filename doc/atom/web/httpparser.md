# HttpHeaderParser Class Documentation

## Overview

The `HttpHeaderParser` class is responsible for parsing and manipulating HTTP headers.

### Constructor

```cpp
HttpHeaderParser();
```

- Constructs a new `HttpHeaderParser` object.

```cpp
HttpHeaderParser parser;
```

---

### Destructor

```cpp
~HttpHeaderParser();
```

- Destroys the `HttpHeaderParser` object.

---

### Method: parseHeaders

```cpp
void parseHeaders(const std::string &rawHeaders);
```

- Parses the raw HTTP headers and stores them internally.

```cpp
std::string rawHeaders = "Content-Type: application/json\r\n";
rawHeaders += "Content-Length: 123\r\n\r\n";
parser.parseHeaders(rawHeaders);
```

---

### Method: setHeaderValue

```cpp
void setHeaderValue(const std::string &key, const std::string &value);
```

- Sets the value of a specific header field.

```cpp
parser.setHeaderValue("Content-Type", "text/html");
```

---

### Method: setHeaders

```cpp
void setHeaders(const std::map<std::string, std::vector<std::string>> &headers);
```

- Sets multiple header fields at once.

```cpp
std::map<std::string, std::vector<std::string>> headers = {
    {"Content-Type", {"application/json"}},
    {"Content-Length", {"123"}}
};
parser.setHeaders(headers);
```

---

### Method: getHeaderValues

```cpp
std::vector<std::string> getHeaderValues(const std::string &key) const;
```

- Retrieves the values of a specific header field.

```cpp
std::vector<std::string> values = parser.getHeaderValues("Content-Type");
for (const auto &value : values) {
    std::cout << "Value: " << value << std::endl;
}
```

---

### Method: removeHeader

```cpp
void removeHeader(const std::string &key);
```

- Removes a specific header field.

```cpp
parser.removeHeader("Content-Length");
```

---

### Method: printHeaders

```cpp
void printHeaders() const;
```

- Prints all the parsed headers to the console.

```cpp
parser.printHeaders();
```

---

### Method: getAllHeaders

```cpp
std::map<std::string, std::vector<std::string>> getAllHeaders() const;
```

- Retrieves all the parsed headers.

```cpp
std::map<std::string, std::vector<std::string>> headers = parser.getAllHeaders();
// Process the retrieved headers
```

---

### Method: hasHeader

```cpp
bool hasHeader(const std::string &key) const;
```

- Checks if a specific header field exists.

```cpp
bool exists = parser.hasHeader("Content-Type");
if (exists) {
    std::cout << "Header exists" << std::endl;
} else {
    std::cout << "Header does not exist" << std::endl;
}
```

---

### Method: clearHeaders

```cpp
void clearHeaders();
```

- Clears all the parsed headers.

```cpp
parser.clearHeaders();
```

---

### Note

- The `HttpHeaderParser` uses the Pimpl idiom, where the implementation details are hidden behind a pointer to implementation (`m_pImpl`). This allows for better encapsulation and reduces compile-time dependencies.

### Complete Example

Here's a complete example of using the `HttpHeaderParser` class:

```cpp
int main() {
    HttpHeaderParser parser;

    std::string rawHeaders = "Content-Type: application/json\r\n";
    rawHeaders += "Content-Length: 123\r\n\r\n";
    parser.parseHeaders(rawHeaders);

    parser.setHeaderValue("Content-Type", "text/html");

    std::vector<std::string> values = parser.getHeaderValues("Content-Type");
    for (const auto &value : values) {
        std::cout << "Value: " << value << std::endl;
    }

    parser.removeHeader("Content-Length");

    parser.printHeaders();

    return 0;
}
```
