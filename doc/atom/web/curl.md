# CurlWrapper Class Documentation

## Introduction

The `CurlWrapper` class is a C++ wrapper for making HTTP requests using the libcurl library. It provides various methods to set request options, perform synchronous and asynchronous requests, and handle response callbacks. This documentation provides detailed information about the class and its usage, along with code examples for each method.

### Class Definition

```cpp
class CurlWrapper {
    // ...
};
```

## Constructor and Destructor

### Constructor

```cpp
/**
 * @brief Constructor for CurlWrapper.
 */
CurlWrapper();
```

#### Usage Example

```cpp
CurlWrapper curl;
```

### Destructor

```cpp
/**
 * @brief Destructor for CurlWrapper.
 */
~CurlWrapper();
```

#### Note

- The destructor automatically cleans up the libcurl handles.

## Setting Request Options

### Set URL

```cpp
/**
 * @brief Set the URL to which the HTTP request will be sent.
 *
 * @param url The URL to set.
 */
void setUrl(const std::string &url);
```

```cpp
curl.setUrl("http://www.example.com/api");
```

### Set Request Method

```cpp
/**
 * @brief Set the request method for the HTTP request (e.g., GET, POST).
 *
 * @param method The request method to set.
 */
void setRequestMethod(const std::string &method);
```

```cpp
curl.setRequestMethod("POST");
```

### Set Header

```cpp
/**
 * @brief Set a header for the HTTP request.
 *
 * @param key The header key.
 * @param value The header value.
 */
void setHeader(const std::string &key, const std::string &value);
```

```cpp
curl.setHeader("Content-Type", "application/json");
```

### Set Timeout

```cpp
/**
 * @brief Set the timeout for the HTTP request.
 *
 * @param timeout The timeout value in seconds.
 */
void setTimeout(long timeout);
```

```cpp
curl.setTimeout(30);
```

### Set Follow Location

```cpp
/**
 * @brief Set whether to follow HTTP redirects automatically.
 *
 * @param follow Boolean value indicating whether to follow redirects.
 */
void setFollowLocation(bool follow);
```

```cpp
curl.setFollowLocation(true);
```

### Set Request Body Data

```cpp
/**
 * @brief Set the request body data for POST requests.
 *
 * @param data The request body data to set.
 */
void setRequestBody(const std::string &data);
```

```cpp
curl.setRequestBody("{ \"key\": \"value\" }");
```

### Set Upload File

```cpp
/**
 * @brief Set the file path for uploading a file in the request.
 *
 * @param filePath The file path to upload.
 */
void setUploadFile(const std::string &filePath);
```

```cpp
curl.setUploadFile("/path/to/file.txt");
```

## Handling Callbacks

### Set On Error Callback

```cpp
/**
 * @brief Set a callback function to handle errors that occur during the request.
 *
 * @param callback The callback function to set.
 */
void setOnErrorCallback(std::function<void(CURLcode)> callback);
```

```cpp
curl.setOnErrorCallback([](CURLcode errorCode) {
    std::cerr << "Error occurred: " << curl_easy_strerror(errorCode) << std::endl;
});
```

### Set On Response Callback

```cpp
/**
 * @brief Set a callback function to handle the response data received from the server.
 *
 * @param callback The callback function to set.
 */
void setOnResponseCallback(std::function<void(const std::string &)> callback);
```

```cpp
curl.setOnResponseCallback([](const std::string &response) {
    std::cout << "Response received: " << response << std::endl;
});
```

## Performing Requests

### Perform Synchronous Request

```cpp
/**
 * @brief Perform a synchronous HTTP request and return the response data.
 *
 * @return The response data received from the server.
 */
std::string performRequest();
```

```cpp
std::string response = curl.performRequest();
std::cout << "Synchronous Response: " << response << std::endl;
```

### Perform Asynchronous Request

```cpp
/**
 * @brief Perform an asynchronous HTTP request and invoke a callback
 * function when the response is received.
 *
 * @param callback The callback function to invoke with the response data.
 */
void asyncPerform(std::function<void(const std::string &)> callback);
```

```cpp
curl.asyncPerform([](const std::string &response) {
    std::cout << "Asynchronous Response: " << response << std::endl;
});
```

### Wait for All Asynchronous Requests

```cpp
/**
 * @brief Wait for all asynchronous requests to complete.
 */
void waitAll();
```

- This method waits for all previously initiated asynchronous requests to complete.

## Internal Implementation

- The following methods and data members are used internally by the `CurlWrapper` class and are not intended for external use.

### Private Data Members

- `CURL *handle`: libcurl easy handle for individual requests
- `CURLM *multiHandle`: libcurl multi handle for managing multiple requests
- `std::vector<std::string> headers`: Vector to store custom headers for the request
- `std::function<void(CURLcode)> onErrorCallback`: Callback function for handling errors
- `std::function<void(const std::string &)> onResponseCallback`: Callback function for handling response data
- `std::mutex mutex`: Mutex for thread safety
- `std::condition_variable cv`: Condition variable for synchronization
- `std::string responseData`: Response data received from the server

### Static Write Callback

```cpp
/**
 * @brief Callback function used by libcurl to write response data into
 * responseData member variable.
 *
 * @param contents Pointer to the response data.
 * @param size Size of each data element.
 * @param nmemb Number of data elements.
 * @param userp User pointer.
 * @return Total size of the data written.
 */
static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *userp);
```

## Overall Example

```cpp
int main() {
    CurlWrapper curl;
    curl.setUrl("http://www.example.com/api");
    curl.setRequestMethod("POST");
    curl.setHeader("Content-Type", "application/json");
    curl.setRequestBody("{ \"key\": \"value\" }");

    // Perform Synchronous Request
    std::string syncResponse = curl.performRequest();
    std::cout << "Synchronous Response: " << syncResponse << std::endl;

    // Perform Asynchronous Request
    curl.asyncPerform([](const std::string &asyncResponse) {
        std::cout << "Asynchronous Response: " << asyncResponse << std::endl;
    });

    // Wait for Asynchronous Request to Complete
    curl.waitAll();

    return 0;
}
```

This example demonstrates setting up a `CurlWrapper` instance, setting request options, performing synchronous and asynchronous requests, and handling the responses.
