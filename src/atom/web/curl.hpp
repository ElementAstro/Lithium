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

namespace atom::web {

/**
 * @brief A comprehensive wrapper class for performing HTTP requests using
 * libcurl.
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

    CurlWrapper(const CurlWrapper &other) = delete;
    auto operator=(const CurlWrapper &other) -> CurlWrapper & = delete;
    CurlWrapper(CurlWrapper &&other) noexcept = delete;
    auto operator=(CurlWrapper &&other) noexcept -> CurlWrapper & = delete;

    /**
     * @brief Set the URL for the HTTP request.
     *
     * @param url The target URL.
     * @return Reference to the CurlWrapper instance.
     */
    auto setUrl(const std::string &url) -> CurlWrapper &;

    /**
     * @brief Set the HTTP request method.
     *
     * @param method HTTP method (e.g., GET, POST).
     * @return Reference to the CurlWrapper instance.
     */
    auto setRequestMethod(const std::string &method) -> CurlWrapper &;

    /**
     * @brief Add a custom header to the HTTP request.
     *
     * @param key Header name.
     * @param value Header value.
     * @return Reference to the CurlWrapper instance.
     */
    auto addHeader(const std::string &key,
                   const std::string &value) -> CurlWrapper &;

    /**
     * @brief Set a callback for handling errors.
     *
     * @param callback Error handling callback.
     * @return Reference to the CurlWrapper instance.
     */
    auto onError(std::function<void(CURLcode)> callback) -> CurlWrapper &;

    /**
     * @brief Set a callback for handling the response data.
     *
     * @param callback Response handling callback.
     * @return Reference to the CurlWrapper instance.
     */
    auto onResponse(std::function<void(const std::string &)> callback)
        -> CurlWrapper &;

    /**
     * @brief Set the timeout for the HTTP request.
     *
     * @param timeout Timeout in seconds.
     * @return Reference to the CurlWrapper instance.
     */
    auto setTimeout(long timeout) -> CurlWrapper &;

    /**
     * @brief Enable or disable following redirects.
     *
     * @param follow True to follow redirects, false otherwise.
     * @return Reference to the CurlWrapper instance.
     */
    auto setFollowLocation(bool follow) -> CurlWrapper &;

    /**
     * @brief Set the request body for POST/PUT requests.
     *
     * @param data Request body data.
     * @return Reference to the CurlWrapper instance.
     */
    auto setRequestBody(const std::string &data) -> CurlWrapper &;

    /**
     * @brief Set the file path for uploading a file.
     *
     * @param filePath Path to the file to upload.
     * @return Reference to the CurlWrapper instance.
     */
    auto setUploadFile(const std::string &filePath) -> CurlWrapper &;

    /**
     * @brief Set proxy settings for the HTTP request.
     *
     * @param proxy Proxy URL.
     * @return Reference to the CurlWrapper instance.
     */
    auto setProxy(const std::string &proxy) -> CurlWrapper &;

    /**
     * @brief Set SSL verification options.
     *
     * @param verifyPeer Enable peer verification.
     * @param verifyHost Enable host verification.
     * @return Reference to the CurlWrapper instance.
     */
    auto setSSLOptions(bool verifyPeer, bool verifyHost) -> CurlWrapper &;

    /**
     * @brief Perform a synchronous HTTP request.
     *
     * @return The response data.
     */
    auto perform() -> std::string;

    /**
     * @brief Perform an asynchronous HTTP request.
     *
     * @return Reference to the CurlWrapper instance.
     */
    auto performAsync() -> CurlWrapper &;

    /**
     * @brief Wait for all asynchronous requests to complete.
     */
    void waitAll();

    /**
     * @brief Set the maximum download speed.
     *
     * @param speed Maximum download speed in bytes per second.
     * @return Reference to the CurlWrapper instance.
     */
    auto setMaxDownloadSpeed(size_t speed) -> CurlWrapper &;

private:
    CURL *handle_;                                   ///< libcurl easy handle
    CURLM *multiHandle_;                             ///< libcurl multi handle
    std::vector<std::string> headers_;               ///< Custom headers
    std::function<void(CURLcode)> onErrorCallback_;  ///< Error callback
    std::function<void(const std::string &)>
        onResponseCallback_;      ///< Response callback
    std::mutex mutex_;            ///< Mutex for thread safety
    std::condition_variable cv_;  ///< Condition variable for synchronization
    std::string responseData_;    ///< Response data

    /**
     * @brief Callback function for writing received data.
     */
    static auto writeCallback(void *contents, size_t size, size_t nmemb,
                              void *userp) -> size_t;
};

}  // namespace atom::web

#endif  // ATOM_WEB_CURL_HPP
