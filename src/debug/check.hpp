#ifndef LITHIUM_DEBUG_CHECK_HPP
#define LITHIUM_DEBUG_CHECK_HPP

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "macro.hpp"

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace lithium::debug {
class CommandChecker {
public:
    enum class ErrorSeverity { WARNING, ERROR, CRITICAL };

    struct Error {
        std::string message;
        size_t line;
        size_t column;
        ErrorSeverity severity;
    } ATOM_ALIGNAS(64);

    struct CheckRule {
        std::string name;
        std::function<std::optional<Error>(const std::string&, size_t)> check;
    } ATOM_ALIGNAS(64);

    CommandChecker();

    void addRule(
        const std::string& name,
        std::function<std::optional<Error>(const std::string&, size_t)> check);

    void setDangerousCommands(const std::vector<std::string>& commands);

    void setMaxLineLength(size_t length);

    ATOM_NODISCARD auto check(std::string_view command) const
        -> std::vector<Error>;

    ATOM_NODISCARD auto toJson(const std::vector<Error>& errors) const -> json;

private:
    std::vector<CheckRule> rules_;
    std::vector<std::string> dangerousCommands_{"rm", "mkfs", "dd", "format"};
    size_t maxLineLength_{80};

    void initializeDefaultRules();

    void checkLine(const std::string& line, size_t lineNumber,
                   std::vector<Error>& errors) const;

    ATOM_NODISCARD auto severityToString(ErrorSeverity severity) const
        -> std::string;
};

void printErrors(const std::vector<CommandChecker::Error>& errors,
                 std::string_view command, bool useColor);
}  // namespace lithium::debug

#endif
