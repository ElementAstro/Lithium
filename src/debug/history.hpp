#ifndef LITHIUM_DEBUG_COMMAND_HISTORY_HPP
#define LITHIUM_DEBUG_COMMAND_HISTORY_HPP

#include <ctime>
#include <memory>
#include <string>
#include <vector>

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
     * @brief Deletes multiple commands by their indices in the history.
     * @param indices The indices of the commands to delete.
     */
    void deleteCommands(const std::vector<size_t>& indices);

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

    /**
     * @brief Exports the command history to a file.
     * @param filePath The path to the file to export to.
     */
    void exportHistory(const std::string& filePath) const;

    /**
     * @brief Gets the frequency of a specific command.
     * @param command The command to get the frequency of.
     * @return The frequency of the command.
     */
    int getCommandFrequency(const std::string& command) const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace lithium::debug

#endif  // LITHIUM_DEBUG_COMMAND_HISTORY_HPP