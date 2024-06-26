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
#include "suggestion.hpp"

#include "addon/manager.hpp"

#include "utils/constant.hpp"

#include <queue>

#include <regex>
#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/string.hpp"

namespace lithium::debug {
ConsoleTerminal::ConsoleTerminal() {
#ifdef _WIN32
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#else
    tcgetattr(STDIN_FILENO, &orig_termios);
#endif
    std::vector<std::string> keywords;
    for (const auto& name : getRegisteredCommands()) {
        keywords.emplace_back(name);
    }
    suggestionEngine = std::make_shared<SuggestionEngine>(std::move(keywords));
    component = std::make_shared<Component>("lithium.terminal");

    component->def("help", &ConsoleTerminal::helpCommand, PointerSentinel(this),
                   "basic", "Show help");
    component->def("list_component", &getComponentList, "component",
                   "Show all components");
    component->def("show_component_info", &getComponentInfo, "component",
                   "Show component info");
}

ConsoleTerminal::~ConsoleTerminal() {
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
#endif
}

std::vector<std::string> ConsoleTerminal::getRegisteredCommands() const {
    std::vector<std::string> commands;
    for (const auto& name : component->getAllCommands()) {
        commands.emplace_back(name);
    }
    return commands;
}

void ConsoleTerminal::callCommand(std::string_view name,
                                  const std::vector<std::any>& args) {
    if (component->has(name.data())) {
        try {
            component->dispatch(name.data(), args);
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << '\n';
        }
    } else {
        std::cout << "Command '" << name << "' not found.\n";
        auto possible_command = suggestionEngine->suggest(std::string(name));
        if (!possible_command.empty()) {
            std::cout << "Did you mean: ";
            for (const auto& cmd : possible_command) {
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
        } else if (command == "clear") {
            clearConsole();
            continue;
        }

        std::string args_str((std::istreambuf_iterator<char>(iss)),
                             std::istreambuf_iterator<char>());
        auto args = parseArguments(args_str);
        callCommand(command, args);
    }
}

std::vector<std::any> ConsoleTerminal::parseArguments(
    const std::string& input) {
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
        } else if (std::isspace(ch) && !inQuotes) {
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

std::any ConsoleTerminal::processToken(const std::string& token) {
    std::regex intRegex("^-?\\d+$");
    std::regex uintRegex("^\\d+u$");
    std::regex longRegex("^-?\\d+l$");
    std::regex ulongRegex("^\\d+ul$");
    std::regex floatRegex("^-?\\d*\\.\\d+f$");
    std::regex doubleRegex("^-?\\d*\\.\\d+$");
    std::regex ldoubleRegex("^-?\\d*\\.\\d+ld$");
    std::regex dateRegex("^\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}$");

    if (token.front() == '"' && token.back() == '"') {
        return token.substr(1, token.size() - 2);  // 去掉引号
    } else if (std::regex_match(token, intRegex)) {
        return std::stoi(token);
    } else if (std::regex_match(token, uintRegex)) {
        return static_cast<unsigned int>(
            std::stoul(token.substr(0, token.size() - 1)));
    } else if (std::regex_match(token, longRegex)) {
        return std::stol(token.substr(0, token.size() - 1));
    } else if (std::regex_match(token, ulongRegex)) {
        return static_cast<unsigned long>(
            std::stoul(token.substr(0, token.size() - 2)));
    } else if (std::regex_match(token, floatRegex)) {
        return std::stof(token.substr(0, token.size() - 1));
    } else if (std::regex_match(token, doubleRegex)) {
        return std::stod(token);
    } else if (std::regex_match(token, ldoubleRegex)) {
        return std::stold(token.substr(0, token.size() - 2));
    } else if (token == "true" || token == "false") {
        return token == "true";
    } else if (std::regex_match(token, dateRegex)) {
        std::tm tm = {};
        std::istringstream ss(token);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
        return tm;
    }
    return token;  // 默认是字符串
}

void ConsoleTerminal::helpCommand(const std::vector<std::string>& args) {
    std::cout << "Available commands:\n";
    for (const auto& cmd : getRegisteredCommands()) {
        std::cout << "  " << cmd << "\n";
    }
}

void ConsoleTerminal::printHeader() {
    std::cout << "Welcome to Lithium Command Line Tool v1.0" << std::endl;
    std::cout << "A debugging tool for Lithium Engine" << std::endl;
    std::cout << "--------------------------------------------------"
              << std::endl;
    std::cout << "Type 'help' to see a list of available commands."
              << std::endl;
    std::cout << "--------------------------------------------------"
              << std::endl;
}

void ConsoleTerminal::clearConsole() {
#ifdef _WIN32
    COORD topLeft = {0, 0};
    CONSOLE_SCREEN_BUFFER_INFO screen;
    DWORD written;
    GetConsoleScreenBufferInfo(hConsole, &screen);
    FillConsoleOutputCharacter(hConsole, ' ', screen.dwSize.X * screen.dwSize.Y,
                               topLeft, &written);
    SetConsoleCursorPosition(hConsole, topLeft);
#else
    std::cout << "\x1B[2J\x1B[H";
#endif
}
}  // namespace lithium::debug
