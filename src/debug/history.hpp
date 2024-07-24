#ifndef LITHIUM_DEBUG_COMMAND_HISTORY_HPP
#define LITHIUM_DEBUG_COMMAND_HISTORY_HPP

#include <ctime>
#include <deque>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>
#include "macro.hpp"

namespace lithium::debug {
/**
 * @brief Manages command history with features like undo, redo, alias
 * management, and filtering.
 */
class CommandHistory {
public:
    /**
     * @brief Constructs a CommandHistory object.
     * @param maxSize The maximum number of commands to keep in history.
     * @param userName The name of the user for file storage purposes.
     */
    CommandHistory(size_t maxSize, std::string userName);

    /**
     * @brief Destructor that saves the command history to a file.
     */
    ~CommandHistory();

    /**
     * @brief Adds a new command to the history.
     * @param command The command to add.
     */
    void addCommand(std::string command);

    /**
     * @brief Undoes the most recent command.
     */
    void undo();

    /**
     * @brief Redoes the most recently undone command.
     */
    void redo();

    /**
     * @brief Prints the command history to the console.
     */
    void printHistory() const;

    /**
     * @brief Searches for commands containing the given keyword.
     * @param keyword The keyword to search for.
     */
    void search(const std::string& keyword) const;

    /**
     * @brief Adds an alias for a command.
     * @param alias The alias name.
     * @param command The command that the alias represents.
     */
    void addAlias(const std::string& alias, const std::string& command);

    /**
     * @brief Executes the command associated with the given alias.
     * @param alias The alias to execute.
     */
    void executeAlias(const std::string& alias);

    /**
     * @brief Deletes a command by its index in the history.
     * @param index The index of the command to delete.
     */
    void deleteCommand(size_t index);

    /**
     * @brief Sorts the command history by timestamp.
     */
    void sortHistoryByTime();

    /**
     * @brief Prints a report of command frequencies.
     */
    void printFrequencyReport() const;

    /**
     * @brief Filters and prints commands within the specified time range.
     * @param startTime The start of the time range.
     * @param endTime The end of the time range.
     */
    void filterHistoryByTime(std::time_t startTime, std::time_t endTime) const;

    /**
     * @brief Clears the command history, redo stack, and frequency report.
     */
    void clearHistory();

private:
    struct CommandEntry {
        std::string command;    ///< The command string.
        std::time_t timestamp;  ///< The time when the command was added.
    } ATOM_ALIGNAS(64);

    size_t maxSize_;        ///< Maximum number of commands to keep in history.
    std::string userName_;  ///< The user name for file storage.
    std::deque<CommandEntry> history_;  ///< The command history.
    std::deque<CommandEntry>
        redoStack_;  ///< The stack of undone commands for redo functionality.
    std::unordered_map<std::string, std::string>
        aliases_;  ///< Aliases for commands.
    std::unordered_map<std::string, int>
        frequency_;  ///< Frequency of each command.

    /**
     * @brief Updates the frequency count of a command.
     * @param command The command to update.
     */
    void updateFrequency(const std::string& command);

    /**
     * @brief Saves the command history, aliases, and frequency to a file.
     */
    void saveToFile() const;

    /**
     * @brief Loads the command history, aliases, and frequency from a file.
     */
    void loadFromFile();
};
}  // namespace lithium::debug

#endif  // LITHIUM_DEBUG_COMMAND_HISTORY_HPP
