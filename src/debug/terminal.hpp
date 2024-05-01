#ifndef LITHIUM_DEBUG_TERMINAL_HPP
#define LITHIUM_DEBUG_TERMINAL_HPP

#include <deque>
#include <fstream>
#include <functional>
#include <iterator>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <iomanip>

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace lithium::Terminal {
namespace fs = std::filesystem;

class ConsoleTerminal {
public:
    using CommandFunction = std::function<void(const std::vector<std::string>&)>;

    ConsoleTerminal();
    ~ConsoleTerminal();

    void registerCommand(std::string_view name, CommandFunction func);

    template <typename Class>
    void registerMemberCommand(std::string_view name, Class* instance, void (Class::*memFunc)(const std::vector<std::string>&)) {
        registerCommand(name, [instance, memFunc](const std::vector<std::string>& args) {
            (instance->*memFunc)(args);
        });
    }

    [[nodiscard]] std::vector<std::string> getRegisteredCommands() const;
    void callCommand(std::string_view name, const std::vector<std::string>& args);
    void run();

protected:
    void helpCommand(const std::vector<std::string>& args);
    void echoCommand(const std::vector<std::string>& args);
    void pwdCommand(const std::vector<std::string>& args);
    void cdCommand(const std::vector<std::string>& args);
    void listDirectory(const std::vector<std::string>& args);
    void createFile(const std::vector<std::string>& args);
    void deleteFile(const std::vector<std::string>& args);
    void createDirectory(const std::vector<std::string>& args);
    void deleteDirectory(const std::vector<std::string>& args);
    void moveFile(const std::vector<std::string>& args);
    void copyFile(const std::vector<std::string>& args);
    void showDateTime(const std::vector<std::string>& args);
    void setDateTime(const std::vector<std::string>& args);

private:
    void printHeader();
    void clearConsole();

    std::unordered_map<std::string, CommandFunction> commandMap;
    static constexpr int MAX_HISTORY_SIZE = 100;

#ifdef _WIN32
    HANDLE hConsole;
#else
    struct termios orig_termios;
#endif
};

} // namespace lithium::Terminal

#endif // LITHIUM_DEBUG_TERMINAL_HPP

