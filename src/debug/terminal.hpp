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

#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

namespace lithium::Terminal {
class ConsoleTerminal {
public:
    using CommandFunction =
        std::function<void(const std::vector<std::string>&)>;

    ConsoleTerminal();

    ~ConsoleTerminal();

    void registerCommand(const std::string& name, CommandFunction func);

    template <typename Class>
    void registerMemberCommand(
        const std::string& name, Class* instance,
        void (Class::*memFunc)(const std::vector<std::string>&)) {
        CommandFunction func = [instance,
                                memFunc](const std::vector<std::string>& args) {
            (instance->*memFunc)(args);
        };
        registerCommand(name, func);
    }

    std::vector<std::string> getRegisteredCommands() const;

    void callCommand(const std::string& name,
                     const std::vector<std::string>& args);

    void run();

protected:
    void helpCommand(const std::vector<std::string>& args);

    void echoCommand(const std::vector<std::string>& args);

    void pwdCommand(const std::vector<std::string>& args);

    void cdCommand(const std::vector<std::string>& args);

    void listDirectory(const std::vector<std::string>& args);

    void createFile(const std::vector<std::string>& args);

    void deleteFile(const std::vector<std::string>& args);

private:
    void printHeader();

private:
    std::unordered_map<std::string, CommandFunction> commandMap;
    static const int MAX_HISTORY_SIZE = 100;

#ifdef _WIN32
    HANDLE hConsole;
#else
    struct termios orig_termios;
#endif

    void clearConsole();
};
}  // namespace lithium::Terminal

#endif
