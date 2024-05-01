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

namespace lithium::Terminal {

ConsoleTerminal::ConsoleTerminal() {
#ifdef _WIN32
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#else
    tcgetattr(STDIN_FILENO, &orig_termios);
#endif
    registerMemberCommand("help", this, &ConsoleTerminal::helpCommand);
    registerMemberCommand("pwd", this, &ConsoleTerminal::pwdCommand);
    registerMemberCommand("echo", this, &ConsoleTerminal::echoCommand);
    registerMemberCommand("create", this, &ConsoleTerminal::createFile);
    registerMemberCommand("delete", this, &ConsoleTerminal::deleteFile);
    registerMemberCommand("cd", this, &ConsoleTerminal::cdCommand);
    registerMemberCommand("ls", this, &ConsoleTerminal::listDirectory);
    registerMemberCommand("mkdir", this, &ConsoleTerminal::createDirectory);
    registerMemberCommand("rmdir", this, &ConsoleTerminal::deleteDirectory);
    registerMemberCommand("mv", this, &ConsoleTerminal::moveFile);
    registerMemberCommand("cp", this, &ConsoleTerminal::copyFile);
    registerMemberCommand("date", this, &ConsoleTerminal::showDateTime);
    registerMemberCommand("setdate", this, &ConsoleTerminal::setDateTime);
}

ConsoleTerminal::~ConsoleTerminal() {
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
#endif
}

void ConsoleTerminal::registerCommand(std::string_view name,
                                      CommandFunction func) {
    commandMap.emplace(name, std::move(func));
}

std::vector<std::string> ConsoleTerminal::getRegisteredCommands() const {
    std::vector<std::string> commands;
    commands.reserve(commandMap.size());
    for (const auto& [name, _] : commandMap) {
        commands.emplace_back(name);
    }
    return commands;
}

void ConsoleTerminal::callCommand(std::string_view name,
                                  const std::vector<std::string>& args) {
    if (auto it = commandMap.find(name.data()); it != commandMap.end()) {
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
    for (const auto& cmd : getRegisteredCommands()) {
        std::cout << "  " << cmd << "\n";
    }
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

void ConsoleTerminal::createDirectory(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "No directory name provided.\n";
        return;
    }
    fs::create_directory(args[0]);
    std::cout << "Directory created: " << args[0] << '\n';
}

void ConsoleTerminal::deleteDirectory(const std::vector<std::string>& args) {
    if (args.empty()) {
        std::cout << "No directory name provided.\n";
        return;
    }
    fs::remove_all(args[0]);
    std::cout << "Directory deleted: " << args[0] << '\n';
}

void ConsoleTerminal::moveFile(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: mv <source> <destination>\n";
        return;
    }
    fs::rename(args[0], args[1]);
    std::cout << "File moved: " << args[0] << " -> " << args[1] << '\n';
}

void ConsoleTerminal::copyFile(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: cp <source> <destination>\n";
        return;
    }
    fs::copy(args[0], args[1]);
    std::cout << "File copied: " << args[0] << " -> " << args[1] << '\n';
}

void ConsoleTerminal::showDateTime(const std::vector<std::string>& args) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current date and time: "
              << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S")
              << '\n';
}

void ConsoleTerminal::setDateTime(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        std::cout << "Usage: setdate <YYYY-MM-DD> <HH:MM:SS>\n";
        return;
    }
    std::tm tm = {};
    std::istringstream ss(args[0] + " " + args[1]);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
    auto time = std::mktime(&tm);
    std::chrono::system_clock::time_point tp =
        std::chrono::system_clock::from_time_t(time);
    std::chrono::system_clock::now() = tp;
    std::cout << "Date and time set to: " << args[0] << " " << args[1] << '\n';
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
