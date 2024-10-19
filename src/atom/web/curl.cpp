/*
 * curl.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-4

Description: Simple HTTP client using libcurl.

**************************************************/

#include "curl.hpp"
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#include <stdexcept>

#include "atom/log/loguru.hpp"

namespace atom::web {
CurlWrapper::CurlWrapper() : multiHandle(curl_multi_init()) {
    LOG_F(INFO, "CurlWrapper constructor called");
    curl_global_init(CURL_GLOBAL_ALL);
    handle = curl_easy_init();
    if (handle == nullptr) {
        LOG_F(ERROR, "Failed to initialize CURL");
        throw std::runtime_error("Failed to initialize CURL.");
    }
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
    LOG_F(INFO, "CurlWrapper initialized successfully");
}

CurlWrapper::~CurlWrapper() {
    LOG_F(INFO, "CurlWrapper destructor called");
    curl_easy_cleanup(handle);
    curl_multi_cleanup(multiHandle);
    curl_global_cleanup();
    LOG_F(INFO, "CurlWrapper cleaned up successfully");
}

void CurlWrapper::setUrl(const std::string &url) {
    LOG_F(INFO, "Setting URL: {}", url);
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
}

void CurlWrapper::setRequestMethod(const std::string &method) {
    LOG_F(INFO, "Setting HTTP method: {}", method);
    if (method == "GET") {
        curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    } else if (method == "POST") {
        curl_easy_setopt(handle, CURLOPT_POST, 1L);
    } else if (method == "PUT") {
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "PUT");
    } else if (method == "DELETE") {
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else if (method == "PATCH") {
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "PATCH");
    } else if (method == "OPTIONS") {
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "OPTIONS");
    } else {
        LOG_F(ERROR, "Unsupported HTTP method: {}", method);
    }
}

void CurlWrapper::setHeader(const std::string &key, const std::string &value) {
    LOG_F(INFO, "Setting HTTP header: {}: {}", key, value);
    headers.push_back(key + ": " + value);
    struct curl_slist *headersList = nullptr;
    for (auto &header : headers) {
        headersList = curl_slist_append(headersList, header.c_str());
    }
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headersList);
}

void CurlWrapper::setOnErrorCallback(std::function<void(CURLcode)> callback) {
    LOG_F(INFO, "Setting onErrorCallback");
    onErrorCallback = std::move(callback);
}

void CurlWrapper::setOnResponseCallback(
    std::function<void(const std::string &)> callback) {
    LOG_F(INFO, "Setting onResponseCallback");
    onResponseCallback = std::move(callback);
}

void CurlWrapper::setTimeout(long timeout) {
    LOG_F(INFO, "Setting timeout: {}", timeout);
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, timeout);
}

void CurlWrapper::setFollowLocation(bool follow) {
    LOG_F(INFO, "Setting follow location: {}", follow ? "true" : "false");
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, follow ? 1L : 0L);
}

void CurlWrapper::setRequestBody(const std::string &data) {
    LOG_F(INFO, "Setting request body: {}", data);
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data.c_str());
}

void CurlWrapper::setUploadFile(const std::string &filePath) {
    LOG_F(INFO, "Setting upload file: {}", filePath);
    curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(handle, CURLOPT_READDATA, fopen(filePath.c_str(), "rb"));
}

auto CurlWrapper::performRequest() -> std::string {
    LOG_F(INFO, "Performing request");
    std::lock_guard<std::mutex> lock(mutex);

    std::string responseData;
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &responseData);

    CURLcode res = curl_easy_perform(handle);
    if (res != CURLE_OK) {
        LOG_F(ERROR, "CURL request failed: {}", curl_easy_strerror(res));
        if (onErrorCallback) {
            onErrorCallback(res);
        }
    } else {
        LOG_F(INFO, "CURL request succeeded");
    }

    if (onResponseCallback) {
        onResponseCallback(responseData);
    }

    return responseData;
}

void CurlWrapper::asyncPerform(
    std::function<void(const std::string &)> callback) {
    LOG_F(INFO, "Performing async request");
    std::unique_lock<std::mutex> lock(mutex);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
    responseData.clear();
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &responseData);
    curl_easy_setopt(handle, CURLOPT_PRIVATE, static_cast<void *>(&callback));
    curl_multi_add_handle(multiHandle, handle);
    lock.unlock();
    cv.notify_one();
}

void CurlWrapper::waitAll() {
    LOG_F(INFO, "Waiting for all async requests to complete");
    CURLMsg *msg;
    int msgsLeft;
    int stillRunning;

    do {
        while (curl_multi_perform(multiHandle, &stillRunning) ==
               CURLM_CALL_MULTI_PERFORM) {
        }

        while ((msg = curl_multi_info_read(multiHandle, &msgsLeft)) !=
               nullptr) {
            if (msg->msg == CURLMSG_DONE) {
                CURL *easyHandle = msg->easy_handle;
                std::function<void(const std::string &)> *callback;
                curl_easy_getinfo(easyHandle, CURLINFO_PRIVATE, &callback);
                if (*callback) {
                    (*callback)(responseData);
                }
                responseData.clear();
                curl_multi_remove_handle(multiHandle, easyHandle);
            } else {
                LOG_F(ERROR,
                      "Error: curl_multi_info_read() returned message with "
                      "unexpected CURLMsg.");
            }
        }
        if (stillRunning != 0) {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock);
        }
    } while (stillRunning != 0);
    LOG_F(INFO, "All async requests completed");
}

auto CurlWrapper::writeCallback(void *contents, size_t size, size_t nmemb,
                                void *userp) -> size_t {
    size_t realsize = size * nmemb;
    auto *str = static_cast<std::string *>(userp);
    str->append(static_cast<char *>(contents), realsize);
    LOG_F(INFO, "Write callback called with size: {}", realsize);
    return realsize;
}
}  // namespace atom::web
