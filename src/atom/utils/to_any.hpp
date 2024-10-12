#ifndef ATOM_UTILS_TO_ANY_HPP
#define ATOM_UTILS_TO_ANY_HPP

#include <any>
#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "atom/error/exception.hpp"

namespace atom::utils {
class ParserException : public atom::error::RuntimeError {
public:
    using atom::error::RuntimeError::RuntimeError;
};

#define THROW_PAESER_ERROR(...)                                           \
    throw ParserException(ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, \
                          __VA_ARGS__)

#define THROW_NESTED_PAESER_ERROR(...)                             \
    ParserException::rethrowNested(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                   ATOM_FUNC_NAME, __VA_ARGS__)

/**
 * @class Parser
 * @brief A class that provides various parsing functionalities.
 *
 * The Parser class offers methods to parse literals, JSON, CSV, and custom
 * types. It also allows registering custom parsers and provides utility
 * functions for printing and logging parsed values.
 */
class Parser {
public:
    /**
     * @brief Type alias for a custom parser function.
     *
     * A custom parser function takes a string as input and returns an optional
     * std::any.
     */
    using CustomParserFunc =
        std::function<std::optional<std::any>(const std::string&)>;

    /**
     * @brief Constructs a new Parser object.
     */
    Parser();

    /**
     * @brief Destroys the Parser object.
     */
    ~Parser();

    /**
     * @brief Parses a literal string into an std::any type.
     *
     * @param input The input string to parse.
     * @return An optional std::any containing the parsed value.
     */
    auto parseLiteral(const std::string& input) -> std::optional<std::any>;

    /**
     * @brief Parses a literal string into an std::any type with a default
     * value.
     *
     * @param input The input string to parse.
     * @param defaultValue The default value to return if parsing fails.
     * @return The parsed value or the default value if parsing fails.
     */
    auto parseLiteralWithDefault(const std::string& input,
                                 const std::any& defaultValue) -> std::any;

    /**
     * @brief Prints the given std::any value.
     *
     * @param value The value to print.
     */
    void print(const std::any& value) const;

    /**
     * @brief Logs the parsing result.
     *
     * @param input The input string that was parsed.
     * @param result The result of the parsing.
     */
    void logParsing(const std::string& input, const std::any& result) const;

    /**
     * @brief Converts a vector of strings to a vector of std::any types.
     *
     * @param input The vector of strings to convert.
     * @return A vector of std::any containing the converted values.
     */
    auto convertToAnyVector(const std::vector<std::string>& input)
        -> std::vector<std::any>;

    /**
     * @brief Registers a custom parser for a specific type.
     *
     * @param type The type for which the custom parser is registered.
     * @param parser The custom parser function.
     */
    void registerCustomParser(const std::string& type, CustomParserFunc parser);

    /**
     * @brief Prints the registered custom parsers.
     */
    void printCustomParsers() const;

    /**
     * @brief Parses a JSON string.
     *
     * @param jsonString The JSON string to parse.
     */
    void parseJson(const std::string& jsonString) const;

    /**
     * @brief Parses a CSV string.
     *
     * @param csvString The CSV string to parse.
     * @param delimiter The delimiter used in the CSV string. Default is ','.
     */
    void parseCsv(const std::string& csvString, char delimiter = ',') const;

private:
    class Impl;  ///< Forward declaration of the implementation class.
    std::unique_ptr<Impl> pImpl_;  ///< Pointer to the implementation.
    std::atomic<bool> isProcessing_{
        false};  ///< Atomic flag indicating if processing is ongoing.
};
}  // namespace atom::utils

#endif