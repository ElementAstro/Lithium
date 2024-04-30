/*
 * terminal.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-30

Description: Terminal

**************************************************/

#include "terminal.hpp"

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

namespace lithium::Terminal {

ConsoleTerminal::ConsoleTerminal() {
#ifdef _WIN32
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#else
    tcgetattr(STDIN_FILENO, &orig_termios);
#endif
    registerMemberCommand("help", this, helpCommand);
    registerMemberCommand("pwd", this, pwdCommand);
    registerMemberCommand("help", this, helpCommand);
    registerMemberCommand("echo", this, echoCommand);
    registerMemberCommand("create", this, createFile);
    registerMemberCommand("delete", this, deleteFile);
}

ConsoleTerminal::~ConsoleTerminal() {
#ifdef _WIN32
    // Windows-specific cleanup can be added here if needed
#else
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
#endif
}

void ConsoleTerminal::registerCommand(const std::string& name,
                                      CommandFunction func) {
    commandMap.emplace(name, std::move(func));
}

std::vector<std::string> ConsoleTerminal::getRegisteredCommands() const {
    std::vector<std::string> commands;
    for (const auto& [name, _] : commandMap) {
        commands.push_back(name);
    }
    return commands;
}

void ConsoleTerminal::callCommand(const std::string& name,
                                  const std::vector<std::string>& args) {
    if (auto it = commandMap.find(name); it != commandMap.end()) {
        try {
            it->second(args);
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << '\n';
        }

    } else {
        std::cout << "Command '" << name << "' not found.\n";
    }
}

void ConsoleTerminal::run() {
    std::string input;
    std::deque<std::string> history;
    int historyIndex = 0;

    printHeader();

    while (true) {
        std::cout << "> ";
        std::getline(std::cin, input);

        history.push_front(input);
        if (history.size() > MAX_HISTORY_SIZE) {
            history.pop_back();
        }
        historyIndex = 0;

        std::istringstream iss(input);
        std::string command;
        iss >> command;
        std::vector<std::string> args(std::istream_iterator<std::string>{iss},
                                      std::istream_iterator<std::string>{});

        if (command == "exit") {
            break;
        } else if (command == "clear") {
            clearConsole();
            continue;
        }

        callCommand(command, args);
    }
}

void ConsoleTerminal::helpCommand(const std::vector<std::string>& args) {
    std::cout << "Available commands:\n";
    std::cout << "  print [args...] - Print arguments to console\n";
    std::cout << "  ls [dir]        - List files in directory\n";
    std::cout << "  rm <file>       - Delete file\n";
    std::cout << "  help            - Show available commands\n";
    std::cout << "  exit            - Exit the terminal\n";
}

void ConsoleTerminal::echoCommand(const std::vector<std::string>& args) {
    for (const auto& arg : args) {
        std::cout << arg << ' ';
    }
    std::cout << '\n';
}

void ConsoleTerminal::pwdCommand(const std::vector<std::string>& args) {
    std::cout << "Current directory: " << fs::current_path() << '\n';
}

void ConsoleTerminal::cdCommand(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "Usage: cd <directory>\n";
        return;
    }
    fs::current_path(args[0]);
}

void ConsoleTerminal::listDirectory(const std::vector<std::string>& args) {
    std::string path = ".";
    if (!args.empty())
        path = args[0];
    for (const auto& entry : fs::directory_iterator(path))
        std::cout << entry.path() << '\n';
}

void ConsoleTerminal::createFile(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "No file name provided.\n";
        return;
    }
    std::ofstream file(args[0]);
    if (file.is_open()) {
        std::cout << "File created: " << args[0] << '\n';
        file.close();
    } else {
        std::cout << "Failed to create file: " << args[0] << '\n';
    }
}

void ConsoleTerminal::deleteFile(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "No file name provided.\n";
        return;
    }
    fs::remove(args[0]);
    std::cout << "File deleted: " << args[0] << '\n';
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

}  // namespace lithium::Terminal
