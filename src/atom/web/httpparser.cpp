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

#include <algorithm>  // C++20 algorithms
#include <iostream>
#include <ranges>  // C++20 ranges
#include <sstream>

namespace atom::web {

class HttpHeaderParser::HttpHeaderParserImpl {
public:
    std::map<std::string, std::vector<std::string>> headers_;
};

HttpHeaderParser::HttpHeaderParser()
    : m_pImpl(std::make_unique<HttpHeaderParser::HttpHeaderParserImpl>()) {}

void HttpHeaderParser::parseHeaders(const std::string &rawHeaders) {
    m_pImpl->headers_.clear();
    std::istringstream iss(rawHeaders);

    std::string line;
    while (std::getline(iss, line)) {
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos) {
            std::string key = line.substr(0, colonPos);
            std::string value = line.substr(colonPos + 1);

            key.erase(key.find_last_not_of(' ') + 1);
            value.erase(0, value.find_first_not_of(' '));

            m_pImpl->headers_[key].push_back(value);
        }
    }
}

void HttpHeaderParser::setHeaderValue(const std::string &key,
                                      const std::string &value) {
    m_pImpl->headers_[key] = {value};
}

void HttpHeaderParser::setHeaders(
    const std::map<std::string, std::vector<std::string>> &headers) {
    m_pImpl->headers_ = headers;
}

void HttpHeaderParser::addHeaderValue(const std::string &key,
                                      const std::string &value) {
    m_pImpl->headers_[key].push_back(value);
}

std::optional<std::vector<std::string>> HttpHeaderParser::getHeaderValues(
    const std::string &key) const {
    if (auto it = m_pImpl->headers_.find(key); it != m_pImpl->headers_.end()) {
        return it->second;
    }
    return std::nullopt;  // Use optional to represent missing values
}

void HttpHeaderParser::removeHeader(const std::string &key) {
    m_pImpl->headers_.erase(key);
}

std::map<std::string, std::vector<std::string>>
HttpHeaderParser::getAllHeaders() const {
    return m_pImpl->headers_;
}

bool HttpHeaderParser::hasHeader(const std::string &key) const {
    return m_pImpl->headers_.contains(key);  // Use C++20 contains method
}

void HttpHeaderParser::clearHeaders() { m_pImpl->headers_.clear(); }

}  // namespace atom::web
