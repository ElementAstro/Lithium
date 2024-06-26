#ifndef LITHIUM_DEBUG_CHECK_HPP
#define LITHIUM_DEBUG_CHECK_HPP

#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "atom/type/json_fwd.hpp"

namespace lithium::debug {
class CommandChecker {
public:
    enum class ErrorSeverity { WARNING, ERROR, CRITICAL };

    struct Error {
        std::string message;
        size_t line;
        size_t column;
        ErrorSeverity severity;
    };

    struct CheckRule {
        std::string name;
        std::function<std::optional<Error>(const std::string&, size_t)> check;
    };

    CommandChecker();

    void addRule(
        const std::string& name,
        std::function<std::optional<Error>(const std::string&, size_t)> check);

    void setDangerousCommands(const std::vector<std::string>& commands);

    void setMaxLineLength(size_t length);

    std::vector<Error> check(std::string_view command) const;

    nlohmann::json toJson(const std::vector<Error>& errors) const;

private:
    std::vector<CheckRule> rules;
    std::vector<std::string> dangerousCommands = {"rm", "mkfs", "dd", "format"};
    size_t maxLineLength = 80;

    void initializeDefaultRules();

    void checkLine(const std::string& line, size_t lineNumber,
                   std::vector<Error>& errors) const;

    std::string severityToString(ErrorSeverity severity) const;
};

void printErrors(const std::vector<CommandChecker::Error>& errors,
                 std::string_view command, bool useColor);
}  // namespace lithium::debug

#endif