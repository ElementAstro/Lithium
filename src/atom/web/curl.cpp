/*
 * curl.cpp
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

#include <condition_variable>
#include <fstream>
#include <stdexcept>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

namespace atom::web {

constexpr long TIMEOUT_MS = 1000;

class CurlWrapper::Impl {
public:
    Impl();
    ~Impl();

    auto setUrl(const std::string &url) -> CurlWrapper::Impl &;
    auto setRequestMethod(const std::string &method) -> CurlWrapper::Impl &;
    auto addHeader(const std::string &key,
                   const std::string &value) -> CurlWrapper::Impl &;
    auto onError(std::function<void(CURLcode)> callback) -> CurlWrapper::Impl &;
    auto onResponse(std::function<void(const std::string &)> callback)
        -> CurlWrapper::Impl &;
    auto setTimeout(long timeout) -> CurlWrapper::Impl &;
    auto setFollowLocation(bool follow) -> CurlWrapper::Impl &;
    auto setRequestBody(const std::string &data) -> CurlWrapper::Impl &;
    auto setUploadFile(const std::string &filePath) -> CurlWrapper::Impl &;
    auto setProxy(const std::string &proxy) -> CurlWrapper::Impl &;
    auto setSSLOptions(bool verifyPeer, bool verifyHost) -> CurlWrapper::Impl &;
    auto perform() -> std::string;
    auto performAsync() -> CurlWrapper::Impl &;
    void waitAll();
    auto setMaxDownloadSpeed(size_t speed) -> CurlWrapper::Impl &;

private:
    CURL *handle_;
    CURLM *multiHandle_;
    std::vector<std::string> headers_;
    std::function<void(CURLcode)> onErrorCallback_;
    std::function<void(const std::string &)> onResponseCallback_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::string responseData_;

    static auto writeCallback(void *contents, size_t size, size_t nmemb,
                              void *userp) -> size_t;
};

CurlWrapper::CurlWrapper() : pImpl_(std::make_unique<Impl>()) {}

CurlWrapper::~CurlWrapper() = default;

auto CurlWrapper::setUrl(const std::string &url) -> CurlWrapper & {
    pImpl_->setUrl(url);
    return *this;
}

auto CurlWrapper::setRequestMethod(const std::string &method) -> CurlWrapper & {
    pImpl_->setRequestMethod(method);
    return *this;
}

auto CurlWrapper::addHeader(const std::string &key,
                            const std::string &value) -> CurlWrapper & {
    pImpl_->addHeader(key, value);
    return *this;
}

auto CurlWrapper::onError(std::function<void(CURLcode)> callback)
    -> CurlWrapper & {
    pImpl_->onError(std::move(callback));
    return *this;
}

auto CurlWrapper::onResponse(std::function<void(const std::string &)> callback)
    -> CurlWrapper & {
    pImpl_->onResponse(std::move(callback));
    return *this;
}

auto CurlWrapper::setTimeout(long timeout) -> CurlWrapper & {
    pImpl_->setTimeout(timeout);
    return *this;
}

auto CurlWrapper::setFollowLocation(bool follow) -> CurlWrapper & {
    pImpl_->setFollowLocation(follow);
    return *this;
}

auto CurlWrapper::setRequestBody(const std::string &data) -> CurlWrapper & {
    pImpl_->setRequestBody(data);
    return *this;
}

auto CurlWrapper::setUploadFile(const std::string &filePath) -> CurlWrapper & {
    pImpl_->setUploadFile(filePath);
    return *this;
}

auto CurlWrapper::setProxy(const std::string &proxy) -> CurlWrapper & {
    pImpl_->setProxy(proxy);
    return *this;
}

auto CurlWrapper::setSSLOptions(bool verifyPeer,
                                bool verifyHost) -> CurlWrapper & {
    pImpl_->setSSLOptions(verifyPeer, verifyHost);
    return *this;
}

auto CurlWrapper::perform() -> std::string { return pImpl_->perform(); }

auto CurlWrapper::performAsync() -> CurlWrapper & {
    pImpl_->performAsync();
    return *this;
}

void CurlWrapper::waitAll() { pImpl_->waitAll(); }

auto CurlWrapper::setMaxDownloadSpeed(size_t speed) -> CurlWrapper & {
    pImpl_->setMaxDownloadSpeed(speed);
    return *this;
}

CurlWrapper::Impl::Impl() : multiHandle_(curl_multi_init()) {
    LOG_F(INFO, "CurlWrapper::Impl constructor called");
    curl_global_init(CURL_GLOBAL_ALL);
    handle_ = curl_easy_init();
    if (handle_ == nullptr) {
        LOG_F(ERROR, "Failed to initialize CURL");
        THROW_CURL_INITIALIZATION_ERROR("Failed to initialize CURL.");
    }
    curl_easy_setopt(handle_, CURLOPT_NOSIGNAL, 1L);
    LOG_F(INFO, "CurlWrapper::Impl initialized successfully");
}

CurlWrapper::Impl::~Impl() {
    LOG_F(INFO, "CurlWrapper::Impl destructor called");
    curl_easy_cleanup(handle_);
    curl_multi_cleanup(multiHandle_);
    curl_global_cleanup();
    LOG_F(INFO, "CurlWrapper::Impl cleaned up successfully");
}

auto CurlWrapper::Impl::setUrl(const std::string &url) -> CurlWrapper::Impl & {
    LOG_F(INFO, "Setting URL: {}", url);
    curl_easy_setopt(handle_, CURLOPT_URL, url.c_str());
    return *this;
}

auto CurlWrapper::Impl::setRequestMethod(const std::string &method)
    -> CurlWrapper::Impl & {
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

auto CurlWrapper::Impl::addHeader(
    const std::string &key, const std::string &value) -> CurlWrapper::Impl & {
    LOG_F(INFO, "Adding header: {}: {}", key, value);
    headers_.emplace_back(key + ": " + value);
    struct curl_slist *headersList = nullptr;
    for (const auto &header : headers_) {
        headersList = curl_slist_append(headersList, header.c_str());
    }
    curl_easy_setopt(handle_, CURLOPT_HTTPHEADER, headersList);
    return *this;
}

auto CurlWrapper::Impl::onError(std::function<void(CURLcode)> callback)
    -> CurlWrapper::Impl & {
    LOG_F(INFO, "Setting onError callback");
    onErrorCallback_ = std::move(callback);
    return *this;
}

auto CurlWrapper::Impl::onResponse(
    std::function<void(const std::string &)> callback) -> CurlWrapper::Impl & {
    LOG_F(INFO, "Setting onResponse callback");
    onResponseCallback_ = std::move(callback);
    return *this;
}

auto CurlWrapper::Impl::setTimeout(long timeout) -> CurlWrapper::Impl & {
    LOG_F(INFO, "Setting timeout: {}", timeout);
    curl_easy_setopt(handle_, CURLOPT_TIMEOUT, timeout);
    return *this;
}

auto CurlWrapper::Impl::setFollowLocation(bool follow) -> CurlWrapper::Impl & {
    LOG_F(INFO, "Setting follow location: {}", follow ? "true" : "false");
    curl_easy_setopt(handle_, CURLOPT_FOLLOWLOCATION, follow ? 1L : 0L);
    return *this;
}

auto CurlWrapper::Impl::setRequestBody(const std::string &data)
    -> CurlWrapper::Impl & {
    LOG_F(INFO, "Setting request body");
    curl_easy_setopt(handle_, CURLOPT_POSTFIELDS, data.c_str());
    return *this;
}

auto CurlWrapper::Impl::setUploadFile(const std::string &filePath)
    -> CurlWrapper::Impl & {
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

auto CurlWrapper::Impl::setProxy(const std::string &proxy)
    -> CurlWrapper::Impl & {
    LOG_F(INFO, "Setting proxy: {}", proxy);
    curl_easy_setopt(handle_, CURLOPT_PROXY, proxy.c_str());
    return *this;
}

auto CurlWrapper::Impl::setSSLOptions(bool verifyPeer,
                                      bool verifyHost) -> CurlWrapper::Impl & {
    LOG_F(INFO, "Setting SSL options: verifyPeer={}, verifyHost={}", verifyPeer,
          verifyHost);
    curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYPEER, verifyPeer ? 1L : 0L);
    curl_easy_setopt(handle_, CURLOPT_SSL_VERIFYHOST, verifyHost ? 2L : 0L);
    return *this;
}

auto CurlWrapper::Impl::perform() -> std::string {
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

auto CurlWrapper::Impl::performAsync() -> CurlWrapper::Impl & {
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

void CurlWrapper::Impl::waitAll() {
    LOG_F(INFO, "Waiting for all asynchronous requests to complete");
    std::unique_lock lock(mutex_);
    cv_.wait(lock);
    LOG_F(INFO, "All asynchronous requests completed");
}

auto CurlWrapper::Impl::writeCallback(void *contents, size_t size, size_t nmemb,
                                      void *userp) -> size_t {
    size_t totalSize = size * nmemb;
    auto *str = static_cast<std::string *>(userp);
    str->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

auto CurlWrapper::Impl::setMaxDownloadSpeed(size_t speed)
    -> CurlWrapper::Impl & {
    curl_easy_setopt(handle_, CURLOPT_MAX_RECV_SPEED_LARGE,
                     static_cast<curl_off_t>(speed));
    return *this;
}

}  // namespace atom::web