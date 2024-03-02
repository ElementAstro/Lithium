/*
 * curl.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Simple HTTP client using libcurl.

**************************************************/

#ifndef ATOM_WEB_CURL_HPP
#define ATOM_WEB_CURL_HPP

#include <curl/curl.h>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <vector>


namespace Atom::Web {
/**
 * @brief A wrapper class for performing HTTP requests using libcurl.
 *
 * This class provides functionality to set various options for the HTTP
 * request, perform synchronous and asynchronous requests, and handle response
 * callbacks. It uses libcurl library for making HTTP requests.
 */
class CurlWrapper {
public:
    /**
     * @brief Constructor for CurlWrapper.
     */
    CurlWrapper();

    /**
     * @brief Destructor for CurlWrapper.
     */
    ~CurlWrapper();

    /**
     * @brief Set the URL to which the HTTP request will be sent.
     *
     * @param url The URL to set.
     */
    void setUrl(const std::string &url);

    /**
     * @brief Set the request method for the HTTP request (e.g., GET, POST).
     *
     * @param method The request method to set.
     */
    void setRequestMethod(const std::string &method);

    /**
     * @brief Set a header for the HTTP request.
     *
     * @param key The header key.
     * @param value The header value.
     */
    void setHeader(const std::string &key, const std::string &value);

    /**
     * @brief Set a callback function to handle errors that occur during the
     * request.
     *
     * @param callback The callback function to set.
     */
    void setOnErrorCallback(std::function<void(CURLcode)> callback);

    /**
     * @brief Set a callback function to handle the response data received from
     * the server.
     *
     * @param callback The callback function to set.
     */
    void setOnResponseCallback(
        std::function<void(const std::string &)> callback);

    /**
     * @brief Set the timeout for the HTTP request.
     *
     * @param timeout The timeout value in seconds.
     */
    void setTimeout(long timeout);

    /**
     * @brief Set whether to follow HTTP redirects automatically.
     *
     * @param follow Boolean value indicating whether to follow redirects.
     */
    void setFollowLocation(bool follow);

    /**
     * @brief Set the request body data for POST requests.
     *
     * @param data The request body data to set.
     */
    void setRequestBody(const std::string &data);

    /**
     * @brief Set the file path for uploading a file in the request.
     *
     * @param filePath The file path to upload.
     */
    void setUploadFile(const std::string &filePath);

    /**
     * @brief Perform a synchronous HTTP request and return the response data.
     *
     * @return The response data received from the server.
     */
    std::string performRequest();

    /**
     * @brief Perform an asynchronous HTTP request and invoke a callback
     * function when the response is received.
     *
     * @param callback The callback function to invoke with the response data.
     */
    void asyncPerform(std::function<void(const std::string &)> callback);

    /**
     * @brief Wait for all asynchronous requests to complete.
     */
    void waitAll();

private:
    CURL *handle;  ///< libcurl easy handle for individual requests
    CURLM
        *multiHandle;  ///< libcurl multi handle for managing multiple requests
    std::vector<std::string>
        headers;  ///< Vector to store custom headers for the request
    std::function<void(CURLcode)>
        onErrorCallback;  ///< Callback function for handling errors
    std::function<void(const std::string &)>
        onResponseCallback;  ///< Callback function for handling response data
    std::mutex mutex;        ///< Mutex for thread safety
    std::condition_variable cv;  ///< Condition variable for synchronization
    std::string responseData;    ///< Response data received from the server

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
    static size_t writeCallback(void *contents, size_t size, size_t nmemb,
                                void *userp);
};

}  // namespace Atom::Web

#endif
