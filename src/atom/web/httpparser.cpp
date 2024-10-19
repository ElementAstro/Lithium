/*
 * httpparser.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-3

Description: Http Header Parser with C++20 features

**************************************************/

#include "httpparser.hpp"

#include <sstream>

#include "atom/log/loguru.hpp"
#include "atom/utils/to_string.hpp"

namespace atom::web {

class HttpHeaderParser::HttpHeaderParserImpl {
public:
    std::map<std::string, std::vector<std::string>> headers;
};

HttpHeaderParser::HttpHeaderParser()
    : m_pImpl(std::make_unique<HttpHeaderParser::HttpHeaderParserImpl>()) {
    LOG_F(INFO, "HttpHeaderParser constructor called");
}

void HttpHeaderParser::parseHeaders(const std::string &rawHeaders) {
    LOG_F(INFO, "parseHeaders called with rawHeaders: {}", rawHeaders);
    m_pImpl->headers.clear();
    std::istringstream iss(rawHeaders);

    std::string line;
    while (std::getline(iss, line)) {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            key.erase(key.find_last_not_of(' ') + 1);
            value.erase(0, value.find_first_not_of(' '));

            m_pImpl->headers[key].push_back(value);
            LOG_F(INFO, "Parsed header: {}: {}", key, value);
        }
    }
    LOG_F(INFO, "parseHeaders completed");
}

void HttpHeaderParser::setHeaderValue(const std::string &key,
                                      const std::string &value) {
    LOG_F(INFO, "setHeaderValue called with key: {}, value: {}", key, value);
    m_pImpl->headers[key] = {value};
    LOG_F(INFO, "Header set: {}: {}", key, value);
}

void HttpHeaderParser::setHeaders(
    const std::map<std::string, std::vector<std::string>> &headers) {
    LOG_F(INFO, "setHeaders called with headers: {}",
          atom::utils::toString(headers));
    m_pImpl->headers = headers;
    LOG_F(INFO, "Headers set successfully");
}

void HttpHeaderParser::addHeaderValue(const std::string &key,
                                      const std::string &value) {
    LOG_F(INFO, "addHeaderValue called with key: {}, value: {}", key, value);
    m_pImpl->headers[key].push_back(value);
    LOG_F(INFO, "Header value added: {}: {}", key, value);
}

auto HttpHeaderParser::getHeaderValues(const std::string &key) const
    -> std::optional<std::vector<std::string>> {
    LOG_F(INFO, "getHeaderValues called with key: {}", key);
    if (auto it = m_pImpl->headers.find(key); it != m_pImpl->headers.end()) {
        LOG_F(INFO, "Header values found for key {}: {}", key,
              atom::utils::toString(it->second));
        return it->second;
    }
    LOG_F(WARNING, "Header values not found for key: {}", key);
    return std::nullopt;  // Use optional to represent missing values
}

void HttpHeaderParser::removeHeader(const std::string &key) {
    LOG_F(INFO, "removeHeader called with key: {}", key);
    m_pImpl->headers.erase(key);
    LOG_F(INFO, "Header removed: {}", key);
}

auto HttpHeaderParser::getAllHeaders() const
    -> std::map<std::string, std::vector<std::string>> {
    LOG_F(INFO, "getAllHeaders called");
    return m_pImpl->headers;
}

auto HttpHeaderParser::hasHeader(const std::string &key) const -> bool {
    LOG_F(INFO, "hasHeader called with key: {}", key);
    bool result = m_pImpl->headers.contains(key);  // Use C++20 contains method
    LOG_F(INFO, "hasHeader result for key {}: {}", key, result);
    return result;
}

void HttpHeaderParser::clearHeaders() {
    LOG_F(INFO, "clearHeaders called");
    m_pImpl->headers.clear();
    LOG_F(INFO, "All headers cleared");
}

}  // namespace atom::web