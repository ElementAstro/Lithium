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
class FileTracker {
public:
    FileTracker(std::string_view directory, std::string_view jsonFilePath,
                std::span<const std::string> fileTypes, bool recursive = false);

    ~FileTracker();

    FileTracker(const FileTracker&) = delete;
    FileTracker& operator=(const FileTracker&) = delete;
    FileTracker(FileTracker&&) noexcept;
    FileTracker& operator=(FileTracker&&) noexcept;

    void scan();
    void compare();
    void logDifferences(std::string_view logFilePath) const;
    void recover(std::string_view jsonFilePath);

    [[nodiscard]] std::future<void> asyncScan();
    [[nodiscard]] std::future<void> asyncCompare();

    [[nodiscard]] const json& getDifferences() const noexcept;
    [[nodiscard]] const std::vector<std::string>& getTrackedFileTypes()
        const noexcept;

    template <std::invocable<const fs::path&> Func>
    void forEachFile(Func&& func) const;

    [[nodiscard]] std::optional<json> getFileInfo(
        const fs::path& filePath) const;

    void addFileType(std::string_view fileType);
    void removeFileType(std::string_view fileType);

    void setEncryptionKey(std::string_view key);

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};
}  // namespace lithium

#endif  // LITHIUM_ADDON_TRACKER_HPP
