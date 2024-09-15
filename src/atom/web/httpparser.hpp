/*
 * httpparser.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-3

Description: Http Header Parser with C++20 features

**************************************************/

#ifndef ATOM_WEB_HTTP_PARSER_HPP
#define ATOM_WEB_HTTP_PARSER_HPP

#include <map>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace atom::web {
/**
 * @brief The HttpHeaderParser class is responsible for parsing and manipulating
 * HTTP headers.
 */
class HttpHeaderParser {
public:
    /**
     * @brief Constructs a new HttpHeaderParser object.
     */
    HttpHeaderParser();

    /**
     * @brief Parses the raw HTTP headers and stores them internally.
     * @param rawHeaders The raw HTTP headers as a string.
     */
    void parseHeaders(const std::string &rawHeaders);

    /**
     * @brief Sets the value of a specific header field.
     * @param key The key of the header field.
     * @param value The value to set.
     */
    void setHeaderValue(const std::string &key, const std::string &value);

    /**
     * @brief Sets multiple header fields at once.
     * @param headers A map containing header fields and their values.
     */
    void setHeaders(
        const std::map<std::string, std::vector<std::string>> &headers);

    /**
     * @brief Adds a new value to an existing header field.
     * @param key The key of the header field.
     * @param value The value to add.
     */
    void addHeaderValue(const std::string &key, const std::string &value);

    /**
     * @brief Retrieves the values of a specific header field.
     * @param key The key of the header field.
     * @return A vector containing the values of the header field.
     */
    [[nodiscard]] auto getHeaderValues(const std::string &key) const
        -> std::optional<std::vector<std::string>>;

    /**
     * @brief Removes a specific header field.
     * @param key The key of the header field to remove.
     */
    void removeHeader(const std::string &key);

    /**
     * @brief Retrieves all the parsed headers.
     * @return A map containing all the parsed headers.
     */
    [[nodiscard]] auto getAllHeaders() const
        -> std::map<std::string, std::vector<std::string>>;

    /**
     * @brief Checks if a specific header field exists.
     * @param key The key of the header field to check.
     * @return True if the header field exists, false otherwise.
     */
    [[nodiscard]] auto hasHeader(const std::string &key) const -> bool;

    /**
     * @brief Clears all the parsed headers.
     */
    void clearHeaders();

private:
    class HttpHeaderParserImpl;
    std::unique_ptr<HttpHeaderParserImpl> m_pImpl;  // Pointer to implementation
};
}  // namespace atom::web

#endif  // ATOM_WEB_HTTP_PARSER_HPP
