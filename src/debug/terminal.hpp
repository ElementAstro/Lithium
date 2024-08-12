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
#include <memory>
#include <string>
#include <vector>
#include "macro.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include "atom/components/component.hpp"

namespace lithium::debug {
class SuggestionEngine;  // Forwards declaration
class CommandChecker;
class CommandHistory;

class ConsoleTerminal {
public:
    ConsoleTerminal();
    ~ConsoleTerminal();

    ATOM_NODISCARD auto getRegisteredCommands() const
        -> std::vector<std::string>;
    void callCommand(std::string_view name, const std::vector<std::any>& args);
    void run();

protected:
    void helpCommand() const;
    void printHistory() const;

private:
    void printHeader();

    auto processToken(const std::string& token) -> std::any;

    auto parseArguments(const std::string& input) -> std::vector<std::any>;

    static auto commandCompletion(const char* text, int start, int end) -> char**;
    static auto commandGenerator(const char* text, int state) -> char*;

    static constexpr int MAX_HISTORY_SIZE = 100;

    std::shared_ptr<SuggestionEngine> suggestionEngine_;

    std::shared_ptr<CommandChecker> commandChecker_;

    std::shared_ptr<CommandHistory> commandHistory_;

    std::shared_ptr<Component> component_;

#ifdef _WIN32
    HANDLE hConsole_;
#else
    struct termios orig_termios_;
#endif
};

extern ConsoleTerminal* globalConsoleTerminal;

}  // namespace lithium::debug

#endif  // LITHIUM_DEBUG_TERMINAL_HPP
