#include "terminal.hpp"

#include <iomanip>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include "atom/macro.hpp"

#if __has_include(<ncurses.h>)
#include <ncurses.h>  // ncurses library
#elif __has_include(<ncurses/ncurses.h>)
#include <ncurses/ncurses.h>
#else
#include <readline/history.h>
#include <readline/readline.h>
#endif

#include "check.hpp"
#include "command.hpp"
#include "history.hpp"
#include "suggestion.hpp"

#include "atom/components/component.hpp"
#include "atom/system/user.hpp"

#include "utils/constant.hpp"

namespace lithium::debug {

auto ctermid() -> std::string {
#ifdef _WIN32
    const int BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    DWORD length = GetConsoleTitleA(buffer, BUFFER_SIZE);
    if (length > 0) {
        return std::string(buffer, length);
    }
#else
    char buffer[L_ctermid];
    if (::ctermid(buffer) != nullptr) {
        return buffer;
    }
#endif
    return "";
}

class ConsoleTerminal::ConsoleTerminalImpl {
public:
    ConsoleTerminalImpl();
    ~ConsoleTerminalImpl();

    [[nodiscard]] auto getRegisteredCommands() const
        -> std::vector<std::string>;
    void callCommand(std::string_view name, const std::vector<std::any>& args);
    void run();

private:
    void initializeNcurses();
    void shutdownNcurses();
    void printHeader();
    auto processToken(const std::string& token) -> std::any;
    auto parseArguments(const std::string& input) -> std::vector<std::any>;

    static auto commandCompletion(const char* text, int start,
                                  int end) -> char**;
    static auto commandGenerator(const char* text, int state) -> char*;

    void handleInput(const std::string& input);

    std::shared_ptr<SuggestionEngine> suggestionEngine_;
    std::shared_ptr<CommandChecker> commandChecker_;
    std::shared_ptr<CommandHistory> commandHistory_;
    std::shared_ptr<Component> component_;

    static constexpr int MAX_HISTORY_SIZE = 100;

    std::vector<std::string> commandHistoryQueue_;
#if __has_include(<ncurses.h>)
    bool ncursesEnabled_;
#endif
};

ConsoleTerminal::ConsoleTerminal()
    : impl_(std::make_unique<ConsoleTerminalImpl>()) {}

ConsoleTerminal::~ConsoleTerminal() = default;

auto ConsoleTerminal::getRegisteredCommands() const
    -> std::vector<std::string> {
    return impl_->getRegisteredCommands();
}

void ConsoleTerminal::callCommand(std::string_view name,
                                  const std::vector<std::any>& args) {
    impl_->callCommand(name, args);
}

void ConsoleTerminal::run() { impl_->run(); }

ConsoleTerminal::ConsoleTerminalImpl::ConsoleTerminalImpl()
    : commandChecker_(std::make_shared<CommandChecker>()),
      commandHistory_(
          std::make_shared<CommandHistory>(64, atom::system::getUsername())) {
    component_ = std::make_shared<Component>("lithium.terminal");

    // Register component commands
    component_->def("quit", &quit, "main", "quit lithium debug terminal");
    component_->def(
        "help",
        [this]() {
            std::cout << "Available commands:\n";
            for (const auto& cmd : getRegisteredCommands()) {
                std::cout << "  " << cmd << "\n";
            }
        },
        "main", "Show help");
    component_->def(
        "history",
        [this]() {
            std::cout << "History:\n";
            commandHistory_->printHistory();
        },
        "main", "Show command history");

    component_->def("load_component", &loadSharedComponent, "component",
                    "Load a shared component");
    component_->def("unload_component", &unloadSharedComponent, "component",
                    "Unload a shared component");
    component_->def("reload_component", &reloadSharedComponent, "component",
                    "Reload a shared component");
    component_->def("scan_component", &scanComponents, "component",
                    "Scan a path");
    component_->def("list_component", &getComponentList, "component",
                    "Show all components");
    component_->def("show_component_info", &getComponentInfo, "component",
                    "Show component info");

    auto registeredCommands = getRegisteredCommands();
    suggestionEngine_ =
        std::make_shared<SuggestionEngine>(std::move(registeredCommands));

    initializeNcurses();
}

ConsoleTerminal::ConsoleTerminalImpl::~ConsoleTerminalImpl() {
    shutdownNcurses();
}

void ConsoleTerminal::ConsoleTerminalImpl::initializeNcurses() {
#if __has_include(<ncurses.h>)
    ncursesEnabled_ = true;
    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
#endif
}

void ConsoleTerminal::ConsoleTerminalImpl::shutdownNcurses() {
#if __has_include(<ncurses.h>)
    if (ncursesEnabled_) {
        endwin();
    }
#endif
}

auto ConsoleTerminal::ConsoleTerminalImpl::getRegisteredCommands() const
    -> std::vector<std::string> {
    std::vector<std::string> commands;
    for (const auto& name : component_->getAllCommands()) {
        commands.push_back(name);
    }
    return commands;
}

void ConsoleTerminal::ConsoleTerminalImpl::callCommand(
    std::string_view name, const std::vector<std::any>& args) {
    if (component_->has(name.data())) {
        try {
            if (!args.empty()) {
                component_->dispatch(name.data(), args);
            } else {
                component_->dispatch(name.data());
            }
        } catch (
            const std::runtime_error& e) {  // Catch a more specific exception
            printw("Error: %s\n", e.what());
        }
        commandHistory_->addCommand(std::string(name));
    } else {
        printw("Command '%s' not found.\n", name.data());
        auto possibleCommand = suggestionEngine_->suggest(name);
        if (!possibleCommand.empty()) {
            printw("Did you mean: \n");
            for (const auto& cmd : possibleCommand) {
                printw("- %s\n", cmd.c_str());
            }
        }
    }
}

void ConsoleTerminal::ConsoleTerminalImpl::run() {
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
#if __has_include(<ncurses.h>) || __has_include(<ncurses/ncurses.h>)
            clear();
#else
            std::cout << "\033[2J\033[1;1H";  // Clear screen for non-ncurses
#endif
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
        if (!args.empty()) {
            for (auto& argument : args) {
                std::cout << "arg: " << argument.type().name() << '\n';
            }
        }
        callCommand(command, args);
    }
}

void ConsoleTerminal::ConsoleTerminalImpl::printHeader() {
#if __has_include(<ncurses.h>) || __has_include(<ncurses/ncurses.h>)
    printw("*** Welcome to Lithium Command Line Tool v1.0 ***\n");
    printw("Type 'help' to see a list of available commands.\n");
#else
    const int BORDER_WIDTH = 60;
    const std::string RESET = "\033[0m";
    const std::string GREEN = "\033[1;32m";
    const std::string BLUE = "\033[1;34m";
    const std::string CYAN = "\033[1;36m";

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
#endif
}

auto ConsoleTerminal::ConsoleTerminalImpl::parseArguments(
    const std::string& input) -> std::vector<std::any> {
    std::vector<std::any> args;
    std::string token;
    bool inQuotes = false;
    std::istringstream iss(input);

    for (char character : input) {
        if (character == '"' && !inQuotes) {
            inQuotes = true;
            if (!token.empty()) {
                args.push_back(processToken(token));
                token.clear();
            }
            token += character;
        } else if (character == '"' && inQuotes) {
            token += character;
            args.push_back(processToken(token));
            token.clear();
            inQuotes = false;
        } else if ((std::isspace(character) != 0) && !inQuotes) {
            if (!token.empty()) {
                args.push_back(processToken(token));
                token.clear();
            }
        } else {
            token += character;
        }
    }

    if (!token.empty()) {
        args.push_back(processToken(token));
    }

    return args;
}

auto ConsoleTerminal::ConsoleTerminalImpl::processToken(
    const std::string& token) -> std::any {
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
        return std::stoul(token.substr(0, token.size() - 2));
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
        std::tm timeStruct = {};
        std::istringstream stream(token);
        stream >> std::get_time(&timeStruct, "%Y-%m-%d %H:%M:%S");
        return timeStruct;
    }
    return token;  // 默认是字符串
}

void ConsoleTerminal::ConsoleTerminalImpl::handleInput(
    const std::string& input) {
    std::istringstream iss(input);
    std::string command;
    iss >> command;
    std::string argsStr(std::istreambuf_iterator<char>(iss), {});

    if (!commandChecker_->check(input).empty()) {
        printw("Command '%s' is dangerous.\n", command.c_str());
        return;
    }
    auto args = parseArguments(argsStr);
    callCommand(command, args);
}

char** ConsoleTerminal::ConsoleTerminalImpl::commandCompletion(
    [[maybe_unused]] const char* text, int start, int end) {
    (void)start;
    (void)end;
#if __has_include(<ncurses.h>) || __has_include(<ncurses/ncurses.h>)
    return nullptr;
#else
    rl_attempted_completion_over = 1;  // Disable default filename completion
    return rl_completion_matches(text, commandGenerator);
#endif
}

char* ConsoleTerminal::ConsoleTerminalImpl::commandGenerator(const char* text,
                                                             int state) {
#if __has_include(<ncurses.h>) || __has_include(<ncurses/ncurses.h>)
    ATOM_UNREF_PARAM(text);
    ATOM_UNREF_PARAM(state);
    return nullptr;
#else
    static std::vector<std::string> matches;
    static size_t matchIndex;

    if (state == 0) {
        matches.clear();
        matchIndex = 0;
        std::string prefix(text);

        auto registeredCommands =
            globalConsoleTerminal->getRegisteredCommands();
        for (const auto& command : registeredCommands) {
            if (command.find(prefix) == 0) {
                matches.push_back(command);
            }
        }
    }

    if (matchIndex < matches.size()) {
        char* result = strdup(matches[matchIndex].c_str());
        matchIndex++;
        return result;
    }
    return nullptr;
#endif
}

ConsoleTerminal* globalConsoleTerminal = nullptr;

}  // namespace lithium::debug
