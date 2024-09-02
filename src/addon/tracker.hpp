#ifndef LITHIUM_ADDON_TRACKER_HPP
#define LITHIUM_ADDON_TRACKER_HPP

#include <memory>
#include <string>
#include <vector>

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

/**
 * @class FileTracker
 * @brief A class that tracks files in a directory, compares their states, logs
 * differences, and can recover files from JSON data.
 *
 * This class provides functionality to scan a directory for files, compare the
 * current state of the files with a previously saved state (in JSON format),
 * log any differences, and recover files based on saved JSON data. It supports
 * both recursive and non-recursive directory scanning and allows tracking
 * specific file types.
 */
class FileTracker {
public:
    /**
     * @brief Constructs a FileTracker instance.
     *
     * @param directory The directory to be tracked.
     * @param jsonFilePath The path to the JSON file where the state of files
     * will be saved.
     * @param fileTypes A vector of file extensions (types) to track.
     * @param recursive A boolean indicating whether to scan directories
     * recursively. Default is false.
     */
    FileTracker(const std::string& directory, const std::string& jsonFilePath,
                const std::vector<std::string>& fileTypes,
                bool recursive = false);

    /**
     * @brief Destructs the FileTracker instance.
     */
    ~FileTracker();

    /**
     * @brief Scans the specified directory for files and generates a JSON file
     * representing the state of the files.
     *
     * This method will create or update the JSON file with the current state of
     * the files in the tracked directory. The state includes details about file
     * names, types, and last modified timestamps.
     */
    void scan();

    /**
     * @brief Compares the new JSON data with the previous state to find
     * differences.
     *
     * This method will read the previously saved JSON file and compare it with
     * the current state of the files. The differences will be stored internally
     * and can be retrieved using the `getDifferences()` method.
     */
    void compare();

    /**
     * @brief Logs the differences between the current and previous file states
     * to a specified log file.
     *
     * @param logFilePath The path to the log file where differences will be
     * written.
     *
     * This method generates a log file detailing the differences detected by
     * the `compare()` method.
     */
    void logDifferences(const std::string& logFilePath);

    /**
     * @brief Recovers files based on the saved JSON data.
     *
     * @param jsonFilePath The path to the JSON file containing the saved state
     * of files.
     *
     * This method will use the provided JSON data to restore files to their
     * state as described in the JSON file. It is useful for recovering lost or
     * changed files based on previous snapshots.
     */
    void recover(const std::string& jsonFilePath);

    /**
     * @brief Retrieves the differences found during the comparison in JSON
     * format.
     *
     * @return A constant reference to a JSON object containing the differences.
     *
     * This method provides access to the JSON representation of differences
     * detected during the `compare()` method.
     */
    [[nodiscard]] auto getDifferences() const -> const json&;

    /**
     * @brief Retrieves the list of tracked file types.
     *
     * @return A constant reference to a vector of file types being tracked.
     *
     * This method returns the list of file extensions that the `FileTracker` is
     * currently monitoring.
     */
    [[nodiscard]] auto getTrackedFileTypes() const
        -> const std::vector<std::string>&;

private:
    // Forward declaration of the implementation class
    struct Impl;

    // Unique pointer to hold the implementation
    std::unique_ptr<Impl> pImpl;
};

#endif  // LITHIUM_ADDON_TRACKER_HPP
