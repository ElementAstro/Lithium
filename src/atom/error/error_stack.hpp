/*
 * error_stack.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Error Stack

**************************************************/

#ifndef ATOM_ERROR_STACK_HPP
#define ATOM_ERROR_STACK_HPP

#include <memory>
#include <ostream>
#include <vector>

#include "atom/macro.hpp"

namespace atom::error {
/**
 * @brief Error information structure.
 */
struct ErrorInfo {
    std::string errorMessage; /**< Error message. */
    std::string moduleName;   /**< Module name. */
    std::string functionName; /**< Function name where the error occurred. */
    int line;                 /**< Line number where the error occurred. */
    std::string fileName;     /**< File name where the error occurred. */
    time_t timestamp;         /**< Timestamp of the error. */
    std::string uuid;         /**< UUID of the error. */
} ATOM_ALIGNAS(128);

/**
 * @brief Overloaded stream insertion operator to print ErrorInfo object.
 * @param os Output stream.
 * @param error ErrorInfo object to be printed.
 * @return Reference to the output stream.
 */
auto operator<<(std::ostream &os, const ErrorInfo &error) -> std::ostream &;

/**
 * @brief Overloaded string concatenation operator to concatenate ErrorInfo
 * object with a string.
 * @param str Input string.
 * @param error ErrorInfo object to be concatenated.
 * @return Concatenated string.
 */
auto operator<<(const std::string &str, const ErrorInfo &error) -> std::string;

/// Represents a stack of errors and provides operations to manage and retrieve
/// them.
class ErrorStack {
    std::vector<ErrorInfo> errorStack_;  ///< The stack of all errors.
    std::vector<ErrorInfo>
        compressedErrorStack_;  ///< The compressed stack of unique errors.
    std::vector<std::string> filteredModules_;  ///< Modules to be filtered out
                                                ///< while printing errors.

public:
    /// Default constructor.
    ErrorStack() = default;

    /// Create a shared pointer to an ErrorStack object.
    /// \return A shared pointer to the ErrorStack object.
    [[nodiscard]] static auto createShared() -> std::shared_ptr<ErrorStack>;

    /// Create a unique pointer to an ErrorStack object.
    /// \return A unique pointer to the ErrorStack object.
    [[nodiscard]] static auto createUnique() -> std::unique_ptr<ErrorStack>;

    /// Insert a new error into the error stack.
    /// \param errorMessage The error message.
    /// \param moduleName The module name where the error occurred.
    /// \param functionName The function name where the error occurred.
    /// \param line The line number where the error occurred.
    /// \param fileName The file name where the error occurred.
    void insertError(const std::string &errorMessage,
                     const std::string &moduleName,
                     const std::string &functionName, int line,
                     const std::string &fileName);

    /// Set the modules to be filtered out while printing the error stack.
    /// \param modules The modules to be filtered out.
    void setFilteredModules(const std::vector<std::string> &modules);

    /// Clear the list of filtered modules.
    void clearFilteredModules();

    /// Print the filtered error stack to the standard output.
    void printFilteredErrorStack() const;

    /// Get a vector of errors filtered by a specific module.
    /// \param moduleName The module name for which errors are to be retrieved.
    /// \return A vector of errors filtered by the given module.
    [[nodiscard]] auto getFilteredErrorsByModule(
        const std::string &moduleName) const -> std::vector<ErrorInfo>;

    /// Get a string containing the compressed errors in the stack.
    /// \return A string containing the compressed errors.
    [[nodiscard]] auto getCompressedErrors() const -> std::string;

private:
    /// Update the compressed error stack based on the current error stack.
    void updateCompressedErrors();

    /// Sort the compressed error stack based on the timestamp of errors.
    void sortCompressedErrorStack();
};
}  // namespace atom::error

#endif
