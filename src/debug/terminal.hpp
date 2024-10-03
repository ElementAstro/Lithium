#ifndef LITHIUM_DEBUG_TERMINAL_HPP
#define LITHIUM_DEBUG_TERMINAL_HPP

#include <any>
#include <memory>
#include <string>
#include <vector>

namespace lithium::debug {

/**
 * @brief Class representing a console terminal for debugging purposes.
 */
class ConsoleTerminal {
public:
    /**
     * @brief Construct a new ConsoleTerminal object.
     */
    ConsoleTerminal();

    /**
     * @brief Destroy the ConsoleTerminal object.
     */
    ~ConsoleTerminal();

    /**
     * @brief Copy constructor (deleted).
     */
    ConsoleTerminal(const ConsoleTerminal&) = delete;

    /**
     * @brief Copy assignment operator (deleted).
     */
    auto operator=(const ConsoleTerminal&) -> ConsoleTerminal& = delete;

    /**
     * @brief Move constructor.
     */
    ConsoleTerminal(ConsoleTerminal&&) noexcept;

    /**
     * @brief Move assignment operator.
     */
    auto operator=(ConsoleTerminal&&) noexcept -> ConsoleTerminal&;

    /**
     * @brief Get the list of registered commands.
     *
     * @return std::vector<std::string> A vector of registered command names.
     */
    [[nodiscard]] auto getRegisteredCommands() const
        -> std::vector<std::string>;

    /**
     * @brief Call a registered command by name with the given arguments.
     *
     * @param name The name of the command to call.
     * @param args A vector of arguments to pass to the command.
     */
    void callCommand(std::string_view name, const std::vector<std::any>& args);

    /**
     * @brief Run the console terminal, processing input and executing commands.
     */
    void run();

private:
    /**
     * @brief Implementation class for ConsoleTerminal.
     *
     * This class is used to hide the implementation details of ConsoleTerminal.
     */
    class ConsoleTerminalImpl;

    std::unique_ptr<ConsoleTerminalImpl>
        impl_;  ///< Pointer to the implementation of ConsoleTerminal.
};

/**
 * @brief Global pointer to the console terminal instance.
 *
 * This pointer can be used to access the console terminal from anywhere in the
 * program.
 */
extern ConsoleTerminal* globalConsoleTerminal;

}  // namespace lithium::debug

#endif  // LITHIUM_DEBUG_TERMINAL_HPP