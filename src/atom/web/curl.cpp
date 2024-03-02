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

namespace Atom::Web {
CurlWrapper::CurlWrapper() {
    curl_global_init(CURL_GLOBAL_ALL);
    handle = curl_easy_init();
    if (!handle) {
        throw std::runtime_error("Failed to initialize CURL.");
    }
    curl_easy_setopt(handle, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0L);
}

CurlWrapper::~CurlWrapper() {
    curl_easy_cleanup(handle);
    curl_global_cleanup();
}

void CurlWrapper::setUrl(const std::string &url) {
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
}

void CurlWrapper::setRequestMethod(const std::string &method) {
    if (method == "GET") {
        DLOG_F(INFO, "HTTP method: GET");
        curl_easy_setopt(handle, CURLOPT_HTTPGET, 1L);
    } else if (method == "POST") {
        DLOG_F(INFO, "HTTP method: POST");
        curl_easy_setopt(handle, CURLOPT_POST, 1L);
    } else if (method == "PUT") {
        DLOG_F(INFO, "HTTP method: PUT");
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "PUT");
    } else if (method == "DELETE") {
        DLOG_F(INFO, "HTTP method: DELETE");
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "DELETE");
    } else if (method == "PATCH") {
        DLOG_F(INFO, "HTTP method: PATCH");
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "PATCH");
    } else if (method == "OPTIONS") {
        DLOG_F(INFO, "HTTP method: OPTIONS");
        curl_easy_setopt(handle, CURLOPT_CUSTOMREQUEST, "OPTIONS");
    } else {
        LOG_F(ERROR, "Unsupported HTTP method: {}", method);
    }
}

void CurlWrapper::setHeader(const std::string &key, const std::string &value) {
    headers.push_back(key + ": " + value);
    struct curl_slist *headersList = nullptr;
    for (auto &header : headers) {
        headersList = curl_slist_append(headersList, header.c_str());
    }
    curl_easy_setopt(handle, CURLOPT_HTTPHEADER, headersList);
    DLOG_F(INFO, "HTTP header: {}", header);
}

void CurlWrapper::setOnErrorCallback(std::function<void(CURLcode)> callback) {
    onErrorCallback = callback;
}

void CurlWrapper::setOnResponseCallback(
    std::function<void(const std::string &)> callback) {
    onResponseCallback = callback;
}

void CurlWrapper::setTimeout(long timeout) {
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, timeout);
}

void CurlWrapper::setFollowLocation(bool follow) {
    curl_easy_setopt(handle, CURLOPT_FOLLOWLOCATION, follow ? 1L : 0L);
    DLOG_F(INFO, "HTTP follow location: {}", follow ? "true" : "false");
}

void CurlWrapper::setRequestBody(const std::string &data) {
    curl_easy_setopt(handle, CURLOPT_POSTFIELDS, data.c_str());
    DLOG_F(INFO, "HTTP request body: {}", data);
}

void CurlWrapper::setUploadFile(const std::string &filePath) {
    curl_easy_setopt(handle, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(handle, CURLOPT_READDATA, fopen(filePath.c_str(), "rb"));
    DLOG_F(INFO, "HTTP upload file: {}", filePath);
}

std::string CurlWrapper::performRequest() {
    std::lock_guard<std::mutex> lock(mutex);

    std::string responseData;
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, &responseData);

    CURLcode res = curl_easy_perform(handle);
    if (res != CURLE_OK && onErrorCallback) {
        onErrorCallback(res);
    }

    if (onResponseCallback) {
        onResponseCallback(responseData);
    }

    return responseData;
}

void CurlWrapper::asyncPerform(
    std::function<void(const std::string &)> callback) {
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
    CURLMsg *msg;
    int msgsLeft;
    int stillRunning;

    do {
        while (curl_multi_perform(multiHandle, &stillRunning) ==
               CURLM_CALL_MULTI_PERFORM)
            ;

        while ((msg = curl_multi_info_read(multiHandle, &msgsLeft))) {
            if (msg->msg == CURLMSG_DONE) {
                CURL *easy_handle = msg->easy_handle;
                std::function<void(const std::string &)> *callback;
                curl_easy_getinfo(easy_handle, CURLINFO_PRIVATE, &callback);
                if (*callback) {
                    (*callback)(responseData);
                }
                responseData.clear();
                curl_multi_remove_handle(multiHandle, easy_handle);
            } else {
                LOG_F(ERROR,
                      "Error: curl_multi_info_read() returned message with "
                      "unexpected CURLMsg.");
            }
        }
        if (stillRunning) {
            std::unique_lock<std::mutex> lock(mutex);
            cv.wait(lock);
        }
    } while (stillRunning);
}

size_t CurlWrapper::writeCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp) {
    size_t realsize = size * nmemb;
    std::string *str = static_cast<std::string *>(userp);
    str->append(static_cast<char *>(contents), realsize);
    return realsize;
}
}  // namespace Atom::Web
