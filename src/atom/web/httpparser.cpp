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

namespace atom::web {

class HttpHeaderParser::HttpHeaderParserImpl {
public:
    std::map<std::string, std::vector<std::string>> headers;
};

HttpHeaderParser::HttpHeaderParser()
    : m_pImpl(std::make_unique<HttpHeaderParser::HttpHeaderParserImpl>()) {}

void HttpHeaderParser::parseHeaders(const std::string &rawHeaders) {
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
        }
    }
}

void HttpHeaderParser::setHeaderValue(const std::string &key,
                                      const std::string &value) {
    m_pImpl->headers[key] = {value};
}

void HttpHeaderParser::setHeaders(
    const std::map<std::string, std::vector<std::string>> &headers) {
    m_pImpl->headers = headers;
}

void HttpHeaderParser::addHeaderValue(const std::string &key,
                                      const std::string &value) {
    m_pImpl->headers[key].push_back(value);
}

auto HttpHeaderParser::getHeaderValues(const std::string &key) const
    -> std::optional<std::vector<std::string>> {
    if (auto it = m_pImpl->headers.find(key); it != m_pImpl->headers.end()) {
        return it->second;
    }
    return std::nullopt;  // Use optional to represent missing values
}

void HttpHeaderParser::removeHeader(const std::string &key) {
    m_pImpl->headers.erase(key);
}

auto HttpHeaderParser::getAllHeaders() const
    -> std::map<std::string, std::vector<std::string>> {
    return m_pImpl->headers;
}

auto HttpHeaderParser::hasHeader(const std::string &key) const -> bool {
    return m_pImpl->headers.contains(key);  // Use C++20 contains method
}

void HttpHeaderParser::clearHeaders() { m_pImpl->headers.clear(); }

}  // namespace atom::web
