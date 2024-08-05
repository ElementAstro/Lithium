/**
 * @file terminal.cpp
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 * @date 2024-5-15
 * @brief Command Terminal
 */

#include "terminal.hpp"
#include "check.hpp"
#include "command.hpp"
#include "console.hpp"
#include "history.hpp"
#include "suggestion.hpp"

#include "addon/manager.hpp"

#include "system/user.hpp"
#include "utils/constant.hpp"

#include <iostream>
#include <memory>
#include <queue>
#include <regex>

#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/string.hpp"

namespace lithium::debug {
ConsoleTerminal::ConsoleTerminal()
    : commandChecker_(std::make_shared<CommandChecker>()),
      commandHistory_(
          std::make_shared<CommandHistory>(64, atom::system::getUsername())) {
#ifdef _WIN32
    hConsole_ = GetStdHandle(STD_OUTPUT_HANDLE);
#else
    tcgetattr(STDIN_FILENO, &orig_termios_);
#endif
    component_ = std::make_shared<Component>("lithium.terminal");
    component_->def("quit", &quit, "main", "quit lithium debug terminal");
    component_->def("help", &ConsoleTerminal::helpCommand,
                    PointerSentinel(this), "main", "Show help");
    component_->def("history", &ConsoleTerminal::printHistory,
                    PointerSentinel(this), "main", "Show command history");

    component_->def("load_component", &loadSharedCompoennt, "component",
                    "Load a shared component");
    component_->def("unload_component", &unloadSharedCompoennt, "component",
                    "Unload a shared component");
    component_->def("reload_component", &reloadSharedCompoennt, "component",
                    "Reload a shared component");
    component_->def("scan_component", &scanComponents, "component",
                    "Scan a path");
    component_->def("list_component", &getComponentList, "component",
                    "Show all components");
    component_->def("show_component_info", &getComponentInfo, "component",
                    "Show component info");
    std::vector<std::string> keywords;
    for (const auto& name : getRegisteredCommands()) {
        keywords.emplace_back(name);
    }
    suggestionEngine_ = std::make_shared<SuggestionEngine>(std::move(keywords));
}

ConsoleTerminal::~ConsoleTerminal() {
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios_);
#endif
}

auto ConsoleTerminal::getRegisteredCommands() const
    -> std::vector<std::string> {
    std::vector<std::string> commands;
    for (const auto& name : component_->getAllCommands()) {
        commands.emplace_back(name);
    }
    return commands;
}

void ConsoleTerminal::callCommand(std::string_view name,
                                  const std::vector<std::any>& args) {
    if (component_->has(name.data())) {
        try {
            if (args.size() != 0) {
                component_->dispatch(name.data(), args);
            } else {
                component_->dispatch(name.data());
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << '\n';
        }
        commandHistory_->addCommand(std::string(name));
    } else {
        std::cout << "Command '" << name << "' not found.\n";
        auto possibleCommand = suggestionEngine_->suggest(name);
        if (!possibleCommand.empty()) {
            std::cout << "Did you mean: ";
            for (const auto& cmd : possibleCommand) {
                std::cout << "- " << cmd << std::endl;
            }
        }
    }
}

void ConsoleTerminal::run() {
    std::string input;
    std::deque<std::string> history;

    printHeader();

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        history.push_front(input);
        if (history.size() > MAX_HISTORY_SIZE) {
            history.pop_back();
        }

        std::istringstream iss(input);
        std::string command;
        iss >> command;

        if (command == "exit") {
            break;
        }
        if (command == "clear") {
            clearScreen();
            continue;
        }

        std::string argsStr((std::istreambuf_iterator<char>(iss)),
                            std::istreambuf_iterator<char>());
        // before parsing, we need to check if the command is dangerous
        if (!commandChecker_->check(input).empty()) {
            std::cout << "Command '" << command << "' is dangerous.\n";
            continue;
        }
        auto args = parseArguments(argsStr);
        callCommand(command, args);
    }
}

auto ConsoleTerminal::parseArguments(const std::string& input)
    -> std::vector<std::any> {
    std::vector<std::any> args;
    std::string token;
    bool inQuotes = false;
    std::istringstream iss(input);

    for (char ch : input) {
        if (ch == '"' && !inQuotes) {
            inQuotes = true;
            if (!token.empty()) {
                args.push_back(processToken(token));
                token.clear();
            }
            token += ch;
        } else if (ch == '"' && inQuotes) {
            token += ch;
            args.push_back(processToken(token));
            token.clear();
            inQuotes = false;
        } else if ((std::isspace(ch) != 0) && !inQuotes) {
            if (!token.empty()) {
                args.push_back(processToken(token));
                token.clear();
            }
        } else {
            token += ch;
        }
    }

    if (!token.empty()) {
        args.push_back(processToken(token));
    }

    return args;
}

auto ConsoleTerminal::processToken(const std::string& token) -> std::any {
    std::regex intRegex("^-?\\d+$");
    std::regex uintRegex("^\\d+u$");
    std::regex longRegex("^-?\\d+l$");
    std::regex ulongRegex("^\\d+ul$");
    std::regex floatRegex(R"(^-?\d*\.\d+f$)");
    std::regex doubleRegex(R"(^-?\d*\.\d+$)");
    std::regex ldoubleRegex(R"(^-?\d*\.\d+ld$)");
    std::regex dateRegex(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$)");

    if (token.front() == '"' && token.back() == '"') {
        return token.substr(1, token.size() - 2);  // 去掉引号
    }
    if (std::regex_match(token, intRegex)) {
        return std::stoi(token);
    }
    if (std::regex_match(token, uintRegex)) {
        return static_cast<unsigned int>(
            std::stoul(token.substr(0, token.size() - 1)));
    }
    if (std::regex_match(token, longRegex)) {
        return std::stol(token.substr(0, token.size() - 1));
    }
    if (std::regex_match(token, ulongRegex)) {
        return static_cast<unsigned long>(
            std::stoul(token.substr(0, token.size() - 2)));
    }
    if (std::regex_match(token, floatRegex)) {
        return std::stof(token.substr(0, token.size() - 1));
    }
    if (std::regex_match(token, doubleRegex)) {
        return std::stod(token);
    }
    if (std::regex_match(token, ldoubleRegex)) {
        return std::stold(token.substr(0, token.size() - 2));
    }
    if (token == "true" || token == "false") {
        return token == "true";
    }
    if (std::regex_match(token, dateRegex)) {
        std::tm tm = {};
        std::istringstream ss(token);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        return tm;
    }
    return token;  // 默认是字符串
}

void ConsoleTerminal::helpCommand() const {
    std::cout << "Available commands:\n";
    for (const auto& cmd : getRegisteredCommands()) {
        std::cout << "  " << cmd << "\n";
    }
}

void ConsoleTerminal::printHistory() const {
    std::cout << "History:\n";
    commandHistory_->printHistory();
}

const std::string RESET = "\033[0m";
const std::string RED = "\033[1;31m";
const std::string GREEN = "\033[1;32m";
const std::string BLUE = "\033[1;34m";
const std::string CYAN = "\033[1;36m";

void ConsoleTerminal::printHeader() {
    const int BORDER_WIDTH = 60;

    // Print top border
    std::cout << BLUE << std::string(BORDER_WIDTH, '*') << RESET << std::endl;

    // Print title
    std::cout << BLUE << "* " << GREEN << std::setw(BORDER_WIDTH - 4)
              << std::left << "Welcome to Lithium Command Line Tool v1.0"
              << " *" << RESET << std::endl;

    // Print description
    std::cout << BLUE << "* " << GREEN << std::setw(BORDER_WIDTH - 4)
              << std::left << "A debugging tool for Lithium Engine" << " *"
              << RESET << std::endl;

    // Print separator line
    std::cout << BLUE << std::string(BORDER_WIDTH, '*') << RESET << std::endl;

    // Print command hint
    std::cout << BLUE << "* " << CYAN << std::setw(BORDER_WIDTH - 4)
              << std::left << "Type 'help' to see a list of available commands."
              << " *" << RESET << std::endl;

    // Print bottom border
    std::cout << BLUE << std::string(BORDER_WIDTH, '*') << RESET << std::endl;
}
}  // namespace lithium::debug
