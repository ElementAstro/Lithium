/**
 * @file terminal.hpp
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 * @date 2024-5-15
 * @brief Command Terminal
 */

#ifndef LITHIUM_DEBUG_TERMINAL_HPP
#define LITHIUM_DEBUG_TERMINAL_HPP

#include <any>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include "atom/components/component.hpp"

namespace lithium::debug {
class SuggestionEngine;  // Forwards declaration
class ConsoleTerminal {
public:
    ConsoleTerminal();
    ~ConsoleTerminal();

    [[nodiscard]] std::vector<std::string> getRegisteredCommands() const;
    void callCommand(std::string_view name,
                                      const std::vector<std::any>& args);
    void run();

protected:
    void helpCommand(const std::vector<std::string>& args);

private:
    void printHeader();
    void clearConsole();

    std::vector<std::any> parseArguments(const std::string& input);

    static constexpr int MAX_HISTORY_SIZE = 100;

    std::shared_ptr<SuggestionEngine> suggestionEngine;

    std::shared_ptr<Component> component;

#ifdef _WIN32
    HANDLE hConsole;
#else
    struct termios orig_termios;
#endif
};

}  // namespace lithium::debug

#endif  // LITHIUM_DEBUG_TERMINAL_HPP
