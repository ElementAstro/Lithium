#ifndef LITHIUM_ADDON_TRACKER_HPP
#define LITHIUM_ADDON_TRACKER_HPP

#include <concepts>
#include <filesystem>
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace fs = std::filesystem;

namespace lithium {

/**
 * @class FileTracker
 * @brief Class to track files in a directory.
 */
class FileTracker {
public:
    /**
     * @brief Constructs a FileTracker.
     * @param directory The directory to track.
     * @param jsonFilePath The path to the JSON file for storing file
     * information.
     * @param fileTypes The types of files to track.
     * @param recursive Whether to track files recursively in subdirectories.
     */
    FileTracker(std::string_view directory, std::string_view jsonFilePath,
                std::span<const std::string> fileTypes, bool recursive = false);

    /**
     * @brief Destructs the FileTracker.
     */
    ~FileTracker();

    // Delete copy constructor and copy assignment operator
    FileTracker(const FileTracker&) = delete;
    FileTracker& operator=(const FileTracker&) = delete;

    /**
     * @brief Move constructor.
     * @param other The other FileTracker to move from.
     */
    FileTracker(FileTracker&&) noexcept;

    /**
     * @brief Move assignment operator.
     * @param other The other FileTracker to move from.
     * @return A reference to this FileTracker.
     */
    FileTracker& operator=(FileTracker&&) noexcept;

    /**
     * @brief Scans the directory for files.
     */
    void scan();

    /**
     * @brief Compares the current state of the directory with the stored state.
     */
    void compare();

    /**
     * @brief Logs the differences between the current and stored state to a
     * file.
     * @param logFilePath The path to the log file.
     */
    void logDifferences(std::string_view logFilePath) const;

    /**
     * @brief Recovers the state of the directory from a JSON file.
     * @param jsonFilePath The path to the JSON file.
     */
    void recover(std::string_view jsonFilePath);

    /**
     * @brief Asynchronously scans the directory for files.
     * @return A future that completes when the scan is done.
     */
    [[nodiscard]] std::future<void> asyncScan();

    /**
     * @brief Asynchronously compares the current state of the directory with
     * the stored state.
     * @return A future that completes when the comparison is done.
     */
    [[nodiscard]] std::future<void> asyncCompare();

    /**
     * @brief Gets the differences between the current and stored state.
     * @return A JSON object containing the differences.
     */
    [[nodiscard]] const json& getDifferences() const noexcept;

    /**
     * @brief Gets the types of files being tracked.
     * @return A vector of strings containing the tracked file types.
     */
    [[nodiscard]] const std::vector<std::string>& getTrackedFileTypes()
        const noexcept;

    /**
     * @brief Applies a function to each file in the directory.
     * @tparam Func The type of the function to apply.
     * @param func The function to apply to each file.
     */
    template <std::invocable<const fs::path&> Func>
    void forEachFile(Func&& func) const;

    /**
     * @brief Gets information about a file.
     * @param filePath The path to the file.
     * @return An optional JSON object containing the file information.
     */
    [[nodiscard]] std::optional<json> getFileInfo(
        const fs::path& filePath) const;

    /**
     * @brief Adds a file type to track.
     * @param fileType The file type to add.
     */
    void addFileType(std::string_view fileType);

    /**
     * @brief Removes a file type from tracking.
     * @param fileType The file type to remove.
     */
    void removeFileType(std::string_view fileType);

    /**
     * @brief Sets the encryption key for storing file information.
     * @param key The encryption key.
     */
    void setEncryptionKey(std::string_view key);

private:
    struct Impl;  ///< Forward declaration of the implementation struct.
    std::unique_ptr<Impl> pImpl;  ///< Pointer to the implementation.
};

}  // namespace lithium

#endif  // LITHIUM_ADDON_TRACKER_HPP