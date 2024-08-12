#include "check.hpp"

#include <iostream>
#include <utility>

#include "atom/type/json.hpp"

namespace lithium::debug {
CommandChecker::CommandChecker() { initializeDefaultRules(); }

void CommandChecker::addRule(
    const std::string& name,
    std::function<std::optional<Error>(const std::string&, size_t)> check) {
    rules_.push_back({name, std::move(check)});
}

void CommandChecker::setDangerousCommands(
    const std::vector<std::string>& commands) {
    dangerousCommands_ = commands;
}

void CommandChecker::setMaxLineLength(size_t length) { maxLineLength_ = length; }

auto CommandChecker::check(
    std::string_view command) const -> std::vector<CommandChecker::Error> {
    std::vector<Error> errors;
    std::vector<std::string> lines;

    std::istringstream iss{std::string(command)};
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    for (size_t i = 0; i < lines.size(); ++i) {
        checkLine(lines[i], i + 1, errors);
    }

    return errors;
}

auto CommandChecker::toJson(const std::vector<Error>& errors) const -> json {
    json j = json::array();
    for (const auto& error : errors) {
        j.push_back({{"message", error.message},
                     {"line", error.line},
                     {"column", error.column},
                     {"severity", severityToString(error.severity)}});
    }
    return j;
}

void CommandChecker::initializeDefaultRules() {
    addRule(
        "forkbomb",
        [](const std::string& line, size_t lineNumber) -> std::optional<Error> {
            auto pos = line.find(":(){ :|:& };:");
            if (pos != std::string::npos) {
                return Error{"Potential forkbomb detected", lineNumber, pos,
                             ErrorSeverity::CRITICAL};
            }
            return std::nullopt;
        });

    addRule("dangerous_commands",
            [this](const std::string& line,
                   size_t lineNumber) -> std::optional<Error> {
                for (const auto& cmd : dangerousCommands_) {
                    auto pos = line.find(cmd);
                    if (pos != std::string::npos) {
                        return Error{"Dangerous command detected: " + cmd,
                                     lineNumber, pos, ErrorSeverity::ERROR};
                    }
                }
                return std::nullopt;
            });

    addRule("line_length",
            [this](const std::string& line,
                   size_t lineNumber) -> std::optional<Error> {
                if (line.length() > maxLineLength_) {
                    return Error{"Line exceeds maximum length", lineNumber,
                                 maxLineLength_, ErrorSeverity::WARNING};
                }
                return std::nullopt;
            });

    addRule(
        "unmatched_quotes",
        [](const std::string& line, size_t lineNumber) -> std::optional<Error> {
            int quoteCount = std::count(line.begin(), line.end(), '"');
            if (quoteCount % 2 != 0) {
                return Error{"Unmatched quotes detected", lineNumber,
                             line.find('"'), ErrorSeverity::ERROR};
            }
            return std::nullopt;
        });

    addRule(
        "backtick_usage",
        [](const std::string& line, size_t lineNumber) -> std::optional<Error> {
            auto pos = line.find('`');
            if (pos != std::string::npos) {
                return Error{
                    "Use of backticks detected, consider using $() instead",
                    lineNumber, pos, ErrorSeverity::WARNING};
            }
            return std::nullopt;
        });
}

void CommandChecker::checkLine(const std::string& line, size_t lineNumber,
                               std::vector<Error>& errors) const {
    for (const auto& rule : rules_) {
        if (auto error = rule.check(line, lineNumber)) {
            errors.push_back(*error);
        }
    }
}

auto CommandChecker::severityToString(ErrorSeverity severity) const -> std::string {
    switch (severity) {
        case ErrorSeverity::WARNING:
            return "warning";
        case ErrorSeverity::ERROR:
            return "error";
        case ErrorSeverity::CRITICAL:
            return "critical";
        default:
            return "unknown";
    }
}

void printErrors(const std::vector<CommandChecker::Error>& errors,
                 std::string_view command, bool useColor) {
    std::vector<std::string> lines;
    std::istringstream iss{std::string(command)};
    std::string line;
    while (std::getline(iss, line)) {
        lines.push_back(line);
    }

    for (const auto& error : errors) {
        std::string severityStr;
        std::string colorCode;
        switch (error.severity) {
            case CommandChecker::ErrorSeverity::WARNING:
                severityStr = "warning";
                colorCode = "\033[33m";
                break;
            case CommandChecker::ErrorSeverity::ERROR:
                severityStr = "error";
                colorCode = "\033[31m";
                break;
            case CommandChecker::ErrorSeverity::CRITICAL:
                severityStr = "CRITICAL";
                colorCode = "\033[35m";
                break;
        }

        if (useColor) {
            std::cout << colorCode;
        }
        std::cout << severityStr << ": " << error.message << "\n";
        std::cout << "  --> line " << error.line << ":" << error.column << "\n";
        std::cout << "   | \n";
        std::cout << " " << error.line << " | " << lines[error.line - 1]
                  << "\n";
        std::cout << "   | " << std::string(error.column, ' ') << "^\n";
        if (useColor) {
            std::cout << "\033[0m";
        }
        std::cout << "\n";
    }
}
}  // namespace lithium::debug
