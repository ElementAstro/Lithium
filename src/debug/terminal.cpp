/**
 * @file terminal.cpp
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 * @date 2024-5-15
 * @brief Command Terminal
 */

#include "terminal.hpp"
#include "command.hpp"
#include "suggestion.hpp"

#include "addon/manager.hpp"

#include "utils/constant.hpp"

#include <queue>

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
    def("help", &ConsoleTerminal::helpCommand,
        PointerSentinel<ConsoleTerminal>(this), "basic", "Show help");
    def("list_component", &getComponentList, "component",
        "Show all components");
    def("show_component_info", &getComponentInfo, "component",
        "Show component info");

    std::vector<std::string> keywords;
    for (const auto& name : getRegisteredCommands()) {
        keywords.emplace_back(name);
    }
    suggestionEngine = std::make_unique<SuggestionEngine>(std::move(keywords));
    commandDispatcher =
        GetWeakPtr<CommandDispatcher>(constants::LITHIUM_COMMAND);
    if (commandDispatcher.expired()) {
        LOG_F(ERROR, "Command dispatcher not found");
    }
}

ConsoleTerminal::~ConsoleTerminal() {
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
#endif
}

std::vector<std::string> ConsoleTerminal::getRegisteredCommands() const {
    std::vector<std::string> commands;
    for (const auto& name : commandDispatcher.lock()->getAllCommands()) {
        commands.emplace_back(name);
    }
    return commands;
}

void ConsoleTerminal::callCommand(std::string_view name,
                                  const std::vector<std::any>& args) {
    if (commandDispatcher.expired()) {
        LOG_F(ERROR, "Command dispatcher not found");
        return;
    }
    if (commandDispatcher.lock()->has(name.data())) {
        try {
            commandDispatcher.lock()->dispatch(name.data(), args);
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
    std::istringstream iss(input);
    std::string token;

    while (iss >> token) {
        if (token.front() == '"' && token.back() == '"') {
            args.push_back(token.substr(1, token.size() - 2));  // 去掉引号
        } else if (token == "true" || token == "false") {
            args.push_back(token == "true");
        } else {
            try {
                size_t pos;
                int int_val = std::stoi(token, &pos);
                if (pos == token.size()) {
                    args.push_back(int_val);
                    continue;
                }
            } catch (...) {
            }

            try {
                size_t pos;
                double double_val = std::stod(token, &pos);
                if (pos == token.size()) {
                    args.push_back(double_val);
                    continue;
                }
            } catch (...) {
            }

            args.push_back(token);  // 默认是字符串
        }
    }

    return args;
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
