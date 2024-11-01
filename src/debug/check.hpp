#ifndef LITHIUM_DEBUG_CHECK_HPP
#define LITHIUM_DEBUG_CHECK_HPP

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "atom/type/json_fwd.hpp"
#include "atom/macro.hpp"
using json = nlohmann::json;

namespace lithium::debug {

/**
 * @brief Class for checking commands against a set of rules.
 */
class CommandChecker {
public:
    /**
     * @brief Enum representing the severity of an error.
     */
    enum class ErrorSeverity {
        WARNING,  ///< Warning level error
        ERROR,    ///< Error level error
        CRITICAL  ///< Critical level error
    };

    /**
     * @brief Struct representing an error found during command checking.
     */
    struct Error {
        std::string message;     ///< The error message
        size_t line;             ///< The line number where the error occurred
        size_t column;           ///< The column number where the error occurred
        ErrorSeverity severity;  ///< The severity of the error
    } ATOM_ALIGNAS(64);

    /**
     * @brief Struct representing a rule for checking commands.
     */
    struct CheckRule {
        std::string name;  ///< The name of the rule
        std::function<std::optional<Error>(const std::string&, size_t)>
            check;  ///< The function to check the rule
    } ATOM_ALIGNAS(64);

    /**
     * @brief Constructs a new CommandChecker object.
     */
    CommandChecker();

    /**
     * @brief Destroys the CommandChecker object.
     */
    ~CommandChecker();

    /**
     * @brief Adds a new rule to the CommandChecker.
     *
     * @param name The name of the rule.
     * @param check The function to check the rule.
     */
    void addRule(
        const std::string& name,
        std::function<std::optional<Error>(const std::string&, size_t)> check);

    /**
     * @brief Sets the list of dangerous commands.
     *
     * @param commands The list of dangerous commands.
     */
    void setDangerousCommands(const std::vector<std::string>& commands);

    /**
     * @brief Sets the maximum allowed line length for commands.
     *
     * @param length The maximum line length.
     */
    void setMaxLineLength(size_t length);

    /**
     * @brief Checks a command against the set rules.
     *
     * @param command The command to check.
     * @return A vector of errors found during the check.
     */
    ATOM_NODISCARD auto check(std::string_view command) const
        -> std::vector<Error>;

    /**
     * @brief Converts a list of errors to JSON format.
     *
     * @param errors The list of errors to convert.
     * @return The JSON representation of the errors.
     */
    ATOM_NODISCARD auto toJson(const std::vector<Error>& errors) const -> json;

private:
    /**
     * @brief Implementation class for CommandChecker.
     */
    class CommandCheckerImpl;
    std::unique_ptr<CommandCheckerImpl>
        impl_;  ///< Pointer to the implementation
};

/**
 * @brief Prints a list of errors to the console.
 *
 * @param errors The list of errors to print.
 * @param command The command that was checked.
 * @param useColor Whether to use color in the output.
 */
void printErrors(const std::vector<CommandChecker::Error>& errors,
                 std::string_view command, bool useColor);

}  // namespace lithium::debug

#endif  // LITHIUM_DEBUG_CHECK_HPP
