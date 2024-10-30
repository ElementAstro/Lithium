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

#ifdef USE_GNUTLS
#include <gnutls/gnutls.h>
#else
#include <openssl/crypto.h>
#include <openssl/ssl.h>
#endif

#include <fstream>
#include <stdexcept>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace atom::web {

constexpr long TIMEOUT_MS = 1000;

CurlWrapper::CurlWrapper() : multiHandle_(curl_multi_init()) {
    LOG_F(INFO, "CurlWrapper constructor called");
    curl_global_init(CURL_GLOBAL_ALL);
    handle_ = curl_easy_init();
    if (handle_ == nullptr) {
        LOG_F(ERROR, "Failed to initialize CURL");
        THROW_CURL_INITIALIZATION_ERROR("Failed to initialize CURL.");
    }
    curl_easy_setopt(handle_, CURLOPT_NOSIGNAL, 1L);
    LOG_F(INFO, "CurlWrapper initialized successfully");
}

CurlWrapper::~CurlWrapper() {
    LOG_F(INFO, "CurlWrapper destructor called");
    curl_easy_cleanup(handle_);
    curl_multi_cleanup(multiHandle_);
    curl_global_cleanup();
    LOG_F(INFO, "CurlWrapper cleaned up successfully");
}

auto CurlWrapper::setUrl(const std::string &url) -> CurlWrapper & {
    LOG_F(INFO, "Setting URL: {}", url);
    curl_easy_setopt(handle_, CURLOPT_URL, url.c_str());
    return *this;
}

auto CurlWrapper::setRequestMethod(const std::string &method) -> CurlWrapper & {
    LOG_F(INFO, "Setting HTTP method: {}", method);
    if (method == "GET") {
        curl_easy_setopt(handle_, CURLOPT_HTTPGET, 1L);
    } else if (method == "POST") {
        curl_easy_setopt(handle_, CURLOPT_POST, 1L);
    } else {
        curl_easy_setopt(handle_, CURLOPT_CUSTOMREQUEST, method.c_str());
    }
    return *this;
}

auto CurlWrapper::addHeader(const std::string &key,
                            const std::string &value) -> CurlWrapper & {
    LOG_F(INFO, "Adding header: {}: {}", key, value);
    headers_.emplace_back(key + ": " + value);
    struct curl_slist *headersList = nullptr;
    for (const auto &header : headers_) {
        headersList = curl_slist_append(headersList, header.c_str());
    }
    curl_easy_setopt(handle_, CURLOPT_HTTPHEADER, headersList);
    return *this;
}

auto CurlWrapper::onError(std::function<void(CURLcode)> callback)
    -> CurlWrapper & {
    LOG_F(INFO, "Setting onError callback");
    onErrorCallback_ = std::move(callback);
    return *this;
}

auto CurlWrapper::onResponse(std::function<void(const std::string &)> callback)
    -> CurlWrapper & {
    LOG_F(INFO, "Setting onResponse callback");
    onResponseCallback_ = std::move(callback);
    return *this;
}

auto CurlWrapper::setTimeout(long timeout) -> CurlWrapper & {
    LOG_F(INFO, "Setting timeout: {}", timeout);
    curl_easy_setopt(handle_, CURLOPT_TIMEOUT, timeout);
    return *this;
}

auto CurlWrapper::setFollowLocation(bool follow) -> CurlWrapper & {
    LOG_F(INFO, "Setting follow location: {}", follow ? "true" : "false");
    curl_easy_setopt(handle_, CURLOPT_FOLLOWLOCATION, follow ? 1L : 0L);
    return *this;
}

auto CurlWrapper::setRequestBody(const std::string &data) -> CurlWrapper & {
    LOG_F(INFO, "Setting request body");
    curl_easy_setopt(handle_, CURLOPT_POSTFIELDS, data.c_str());
    return *this;
}

auto CurlWrapper::setUploadFile(const std::string &filePath) -> CurlWrapper & {
    LOG_F(INFO, "Setting upload file: {}", filePath);
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        LOG_F(ERROR, "Failed to open file: {}", filePath);
        throw std::runtime_error("Failed to open file for upload.");
    }
    curl_easy_setopt(handle_, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(handle_, CURLOPT_READDATA, file.rdbuf());
    return *this;
}

auto CurlWrapper::setProxy(const std::string &proxy) -> CurlWrapper & {
    LOG_F(INFO, "Setting proxy: {}", proxy);
    curl_easy_setopt(handle_, CURLOPT_PROXY, proxy.c_str());
    return *this;
}

auto CurlWrapper::setSSLOptions(bool verifyPeer,
                                bool verifyHost) -> CurlWrapper & {
    LOG_F(INFO, "Setting SSL options: verifyPeer={}, verifyHost={}", verifyPeer,
          verifyHost);
    curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYPEER, verifyPeer ? 1L : 0L);
    curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYHOST, verifyHost ? 2L : 0L);
    return *this;
}

std::string CurlWrapper::perform() {
    LOG_F(INFO, "Performing synchronous request");
    std::lock_guard lock(mutex_);
    responseData_.clear();

    curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(handle_, CURLOPT_WRITEDATA, &responseData_);

    CURLcode res = curl_easy_perform(handle_);
    if (res != CURLE_OK) {
        LOG_F(ERROR, "CURL request failed: {}", curl_easy_strerror(res));
        if (onErrorCallback_) {
            onErrorCallback_(res);
        }
        THROW_CURL_RUNTIME_ERROR("CURL perform failed.");
    }

    if (onResponseCallback_) {
        onResponseCallback_(responseData_);
    }

    return responseData_;
}

auto CurlWrapper::performAsync() -> CurlWrapper & {
    LOG_F(INFO, "Performing asynchronous request");
    std::lock_guard lock(mutex_);
    responseData_.clear();

    curl_easy_setopt(handle_, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(handle_, CURLOPT_WRITEDATA, &responseData_);

    CURLMcode multiCode = curl_multi_add_handle(multiHandle_, handle_);
    if (multiCode != CURLM_OK) {
        LOG_F(ERROR, "curl_multi_add_handle failed: {}",
              curl_multi_strerror(multiCode));
        THROW_CURL_RUNTIME_ERROR("Failed to add handle to multi handle.");
    }

    // Start a separate thread to handle the multi interface
    std::thread([this]() {
        int stillRunning = 0;
        curl_multi_perform(multiHandle_, &stillRunning);

        while (stillRunning != 0) {
            int numfds;
            CURLMcode multiCode =
                curl_multi_wait(multiHandle_, nullptr, 0, TIMEOUT_MS, &numfds);
            if (multiCode != CURLM_OK) {
                LOG_F(ERROR, "curl_multi_wait failed: {}",
                      curl_multi_strerror(multiCode));
                break;
            }
            curl_multi_perform(multiHandle_, &stillRunning);
        }

        CURLMsg *msg;
        int msgsLeft;
        while ((msg = curl_multi_info_read(multiHandle_, &msgsLeft)) !=
               nullptr) {
            if (msg->msg == CURLMSG_DONE) {
                CURL *easyHandle = msg->easy_handle;
                char *url = nullptr;
                curl_easy_getinfo(easyHandle, CURLINFO_EFFECTIVE_URL, &url);
                LOG_F(INFO, "Completed request: {}", url ? url : "unknown");

                if (msg->data.result != CURLE_OK) {
                    LOG_F(ERROR, "Async request failed: {}",
                          curl_easy_strerror(msg->data.result));
                    if (onErrorCallback_) {
                        onErrorCallback_(msg->data.result);
                    }
                } else {
                    if (onResponseCallback_) {
                        onResponseCallback_(responseData_);
                    }
                }

                curl_multi_remove_handle(multiHandle_, easyHandle);
            }
        }

        cv_.notify_one();
    }).detach();

    return *this;
}

void CurlWrapper::waitAll() {
    LOG_F(INFO, "Waiting for all asynchronous requests to complete");
    std::unique_lock lock(mutex_);
    cv_.wait(lock);
    LOG_F(INFO, "All asynchronous requests completed");
}

auto CurlWrapper::writeCallback(void *contents, size_t size, size_t nmemb,
                                void *userp) -> size_t {
    size_t totalSize = size * nmemb;
    auto *str = static_cast<std::string *>(userp);
    str->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

auto CurlWrapper::setMaxDownloadSpeed(size_t speed) -> CurlWrapper & {
    curl_easy_setopt(handle_, CURLOPT_MAX_RECV_SPEED_LARGE,
                     static_cast<curl_off_t>(speed));
    return *this;
}
}  // namespace atom::web