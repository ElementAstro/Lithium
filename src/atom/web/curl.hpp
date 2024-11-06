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
#include <functional>
#include <memory>
#include <string>

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

    auto setUrl(const std::string &url) -> CurlWrapper &;
    auto setRequestMethod(const std::string &method) -> CurlWrapper &;
    auto addHeader(const std::string &key,
                   const std::string &value) -> CurlWrapper &;
    auto onError(std::function<void(CURLcode)> callback) -> CurlWrapper &;
    auto onResponse(std::function<void(const std::string &)> callback)
        -> CurlWrapper &;
    auto setTimeout(long timeout) -> CurlWrapper &;
    auto setFollowLocation(bool follow) -> CurlWrapper &;
    auto setRequestBody(const std::string &data) -> CurlWrapper &;
    auto setUploadFile(const std::string &filePath) -> CurlWrapper &;
    auto setProxy(const std::string &proxy) -> CurlWrapper &;
    auto setSSLOptions(bool verifyPeer, bool verifyHost) -> CurlWrapper &;
    auto perform() -> std::string;
    auto performAsync() -> CurlWrapper &;
    void waitAll();
    auto setMaxDownloadSpeed(size_t speed) -> CurlWrapper &;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl_;
};

}  // namespace atom::web

#endif  // ATOM_WEB_CURL_HPP
