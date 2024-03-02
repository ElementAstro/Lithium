/*
 * httpparser.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-11-3

Description: Http Header Parser

**************************************************/

#ifndef ATOM_WEB_HTTP_PARSER_HPP
#define ATOM_WEB_HTTP_PARSER_HPP

#include <map>
#include <memory>
#include <string>
#include <vector>


namespace Atom::Web {
class HttpHeaderParserImpl;

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
     * @brief Destroys the HttpHeaderParser object.
     */
    ~HttpHeaderParser();

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
     * @brief Retrieves the values of a specific header field.
     * @param key The key of the header field.
     * @return A vector containing the values of the header field.
     */
    std::vector<std::string> getHeaderValues(const std::string &key) const;

    /**
     * @brief Removes a specific header field.
     * @param key The key of the header field to remove.
     */
    void removeHeader(const std::string &key);

    /**
     * @brief Prints all the parsed headers to the console.
     */
    void printHeaders() const;

    /**
     * @brief Retrieves all the parsed headers.
     * @return A map containing all the parsed headers.
     */
    std::map<std::string, std::vector<std::string>> getAllHeaders() const;

    /**
     * @brief Checks if a specific header field exists.
     * @param key The key of the header field to check.
     * @return True if the header field exists, false otherwise.
     */
    bool hasHeader(const std::string &key) const;

    /**
     * @brief Clears all the parsed headers.
     */
    void clearHeaders();

private:
    std::unique_ptr<HttpHeaderParserImpl> m_pImpl;  // Pointer to implementation
};
}  // namespace Atom::Web

#endif  // ATOM_WEB_HTTP_PARSER_HPP