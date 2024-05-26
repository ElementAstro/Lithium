/*
 * httpparser.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-3

Description: Http Header Parser

**************************************************/

#include "httpparser.hpp"

#include <iostream>
#include <sstream>

namespace atom::web {
class HttpHeaderParserImpl {
public:
    std::map<std::string, std::vector<std::string>> headers_;
};

HttpHeaderParser::HttpHeaderParser() : m_pImpl(std::make_unique<HttpHeaderParserImpl>()) {}

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

std::vector<std::string> HttpHeaderParser::getHeaderValues(
    const std::string &key) const {
    auto it = m_pImpl->headers_.find(key);
    if (it != m_pImpl->headers_.end()) {
        return it->second;
    } else {
        return {};
    }
}

void HttpHeaderParser::removeHeader(const std::string &key) {
    m_pImpl->headers_.erase(key);
}

void HttpHeaderParser::printHeaders() const {
    for (const auto &entry : m_pImpl->headers_) {
        std::cout << entry.first << ": ";
        for (const auto &value : entry.second) {
            std::cout << value << ", ";
        }
        std::cout << std::endl;
    }
}

std::map<std::string, std::vector<std::string>>
HttpHeaderParser::getAllHeaders() const {
    return m_pImpl->headers_;
}

bool HttpHeaderParser::hasHeader(const std::string &key) const {
    return m_pImpl->headers_.find(key) != m_pImpl->headers_.end();
}

void HttpHeaderParser::clearHeaders() { m_pImpl->headers_.clear(); }

}  // namespace atom::web
