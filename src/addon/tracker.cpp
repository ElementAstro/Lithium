#include "tracker.hpp"

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iomanip>
#include <mutex>
#include <optional>
#include <queue>
#include <shared_mutex>
#include <thread>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/aes.hpp"
#include "atom/utils/difflib.hpp"
#include "atom/utils/string.hpp"
#include "atom/utils/time.hpp"

namespace lithium {
class FailToScanDirectory : public atom::error::Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_SCAN_DIRECTORY(...)                              \
    throw lithium::FailToScanDirectory(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                       ATOM_FUNC_NAME, __VA_ARGS__)

#define THROW_NESTED_FAIL_TO_SCAN_DIRECTORY(...) \
    lithium::FailToScanDirectory::rethrowNested( \
        ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, __VA_ARGS__)

class FailToCompareJSON : public atom::error::Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_COMPARE_JSON(...)                              \
    throw lithium::FailToCompareJSON(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                     ATOM_FUNC_NAME, __VA_ARGS__)

#define THROW_NESTED_FAIL_TO_COMPARE_JSON(...)                                \
    lithium::FailToCompareJSON::rethrowNested(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                              ATOM_FUNC_NAME, __VA_ARGS__)

class FailToLogDifferences : public atom::error::Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_LOG_DIFFERENCES(...)                              \
    throw lithium::FailToLogDifferences(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                        ATOM_FUNC_NAME, __VA_ARGS__)

#define THROW_NESTED_FAIL_TO_LOG_DIFFERENCES(...) \
    lithium::FailToLogDifferences::rethrowNested( \
        ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, __VA_ARGS__)

class FailToRecoverFiles : public atom::error::Exception {
public:
    using Exception::Exception;
};

#define THROW_FAIL_TO_RECOVER_FILES(...)                              \
    throw lithium::FailToRecoverFiles(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                      ATOM_FUNC_NAME, __VA_ARGS__)

#define THROW_NESTED_FAIL_TO_RECOVER_FILES(...)                                \
    lithium::FailToRecoverFiles::rethrowNested(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                               ATOM_FUNC_NAME, __VA_ARGS__)

struct FileTracker::Impl {
    std::string directory;
    std::string jsonFilePath;
    bool recursive;
    std::vector<std::string> fileTypes;
    json newJson;
    json oldJson;
    json differences;
    std::shared_mutex mtx;
    std::optional<std::string> encryptionKey;

    // Thread pool members
    std::vector<std::thread> threadPool;
    std::queue<std::function<void()>> tasks;
    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

    Impl(std::string_view dir, std::string_view jFilePath,
         std::span<const std::string> types, bool rec)
        : directory(dir),
          jsonFilePath(jFilePath),
          recursive(rec),
          fileTypes(types.begin(), types.end()),
          stop(false) {
        // Initialize thread pool with hardware concurrency
        size_t threadCount = std::thread::hardware_concurrency();
        for (size_t i = 0; i < threadCount; ++i) {
            threadPool.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queueMutex);
                        condition.wait(
                            lock, [this]() { return stop || !tasks.empty(); });
                        if (stop && tasks.empty())
                            return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    try {
                        task();
                    } catch (const std::exception& e) {
                        // Log or handle task exceptions
                        // For simplicity, we'll ignore here
                    }
                }
            });
        }
    }

    ~Impl() {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& thread : threadPool) {
            if (thread.joinable()) {
                thread.join();
            }
        }
    }

    void enqueueTask(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            tasks.emplace(std::move(task));
        }
        condition.notify_one();
    }

    static void saveJSON(const json& j, const std::string& filePath,
                         const std::optional<std::string>& key) {
        std::ofstream outFile(filePath, std::ios::binary);
        if (!outFile.is_open()) {
            THROW_FAIL_TO_OPEN_FILE("Failed to open file for writing: " +
                                    filePath);
        }
        if (key) {
            std::vector<unsigned char> iv;
            std::vector<unsigned char> tag;
            std::string encrypted =
                atom::utils::encryptAES(j.dump(), *key, iv, tag);
            outFile.write(encrypted.data(), encrypted.size());
        } else {
            outFile << std::setw(4) << j << std::endl;
        }
    }

    static auto loadJSON(const std::string& filePath,
                         const std::optional<std::string>& key) -> json {
        std::ifstream inFile(filePath, std::ios::binary);
        if (!inFile.is_open()) {
            return {};
        }
        if (key) {
            std::string encrypted((std::istreambuf_iterator<char>(inFile)),
                                  std::istreambuf_iterator<char>());
            std::vector<unsigned char> iv;
            std::vector<unsigned char> tag;
            std::string decrypted =
                atom::utils::decryptAES(encrypted, *key, iv, tag);
            return json::parse(decrypted);
        }
        json j;
        inFile >> j;
        return j;
    }

    void generateJSON() {
        using DirIterVariant =
            std::variant<std::filesystem::directory_iterator,
                         std::filesystem::recursive_directory_iterator>;

        DirIterVariant fileRange =
            recursive
                ? DirIterVariant(
                      std::filesystem::recursive_directory_iterator(directory))
                : DirIterVariant(
                      std::filesystem::directory_iterator(directory));

        std::visit(
            [&](auto&& iter) {
                for (const auto& entry : iter) {
                    if (std::ranges::find(fileTypes,
                                          entry.path().extension().string()) !=
                        fileTypes.end()) {
                        enqueueTask(
                            [this, entry]() { processFile(entry.path()); });
                    }
                }
            },
            fileRange);

        // Wait for all tasks to finish
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            condition.wait(lock, [this]() { return tasks.empty(); });
        }

        saveJSON(newJson, jsonFilePath, encryptionKey);
    }

    void processFile(const std::filesystem::path& entry) {
        try {
            std::string hash = atom::utils::calculateSha256(entry.string());
            std::string lastWriteTime = atom::utils::getChinaTimestampString();

            std::unique_lock lock(mtx);
            newJson[entry.string()] = {
                {"last_write_time", lastWriteTime},
                {"hash", hash},
                {"size", std::filesystem::file_size(entry)},
                {"type", entry.extension().string()}};
        } catch (const std::exception& e) {
            // Handle file processing exceptions
            // For simplicity, we'll ignore here
        }
    }

    auto compareJSON() -> json {
        json diff;
        for (const auto& [filePath, newFileInfo] : newJson.items()) {
            if (oldJson.contains(filePath)) {
                if (oldJson[filePath]["hash"] != newFileInfo["hash"]) {
                    // 使用 difflib 生成详细的差异
                    std::vector<std::string> oldLines =
                        atom::utils::splitString(oldJson[filePath].dump(),
                                                 '\n');
                    std::vector<std::string> newLines =
                        atom::utils::splitString(newFileInfo.dump(), '\n');
                    auto differences = atom::utils::Differ::unifiedDiff(
                        oldLines, newLines, "old", "new");
                    diff[filePath] = {{"status", "modified"},
                                      {"diff", differences}};
                }
            } else {
                diff[filePath] = {{"status", "new"}};
            }
        }
        for (const auto& [filePath, oldFileInfo] : oldJson.items()) {
            if (!newJson.contains(filePath)) {
                diff[filePath] = {{"status", "deleted"}};
            }
        }
        return diff;
    }

    void recoverFiles() {
        for (const auto& [filePath, fileInfo] : oldJson.items()) {
            if (!std::filesystem::exists(filePath)) {
                try {
                    std::ofstream outFile(filePath);
                    if (outFile.is_open()) {
                        outFile << "This file was recovered based on version: "
                                << fileInfo["last_write_time"] << std::endl;
                        outFile.close();
                    }
                } catch (const std::exception& e) {
                    // Handle recovery exceptions
                    // For simplicity, we'll ignore here
                }
            }
        }
    }
};

FileTracker::FileTracker(std::string_view directory,
                         std::string_view jsonFilePath,
                         std::span<const std::string> fileTypes, bool recursive)
    : pImpl(std::make_unique<Impl>(directory, jsonFilePath, fileTypes,
                                   recursive)) {}

FileTracker::~FileTracker() = default;

FileTracker::FileTracker(FileTracker&&) noexcept = default;
auto FileTracker::operator=(FileTracker&&) noexcept -> FileTracker& = default;

void FileTracker::scan() {
    try {
        if (std::filesystem::exists(pImpl->jsonFilePath)) {
            pImpl->oldJson =
                pImpl->loadJSON(pImpl->jsonFilePath, pImpl->encryptionKey);
        }
        pImpl->generateJSON();
    } catch (const std::exception& e) {
        // Handle scan exceptions
        THROW_FAIL_TO_SCAN_DIRECTORY("Scan failed: " + std::string(e.what()));
    }
}

void FileTracker::compare() {
    try {
        pImpl->differences = pImpl->compareJSON();
    } catch (const std::exception& e) {
        // Handle compare exceptions
        THROW_FAIL_TO_COMPARE_JSON("Compare failed: " + std::string(e.what()));
    }
}

void FileTracker::logDifferences(std::string_view logFilePath) const {
    try {
        std::ofstream logFile(logFilePath.data(), std::ios_base::app);
        if (!logFile.is_open()) {
            THROW_FAIL_TO_OPEN_FILE("Failed to open log file: " +
                                    std::string(logFilePath));
        }
        for (const auto& [filePath, info] : pImpl->differences.items()) {
            logFile << "File: " << filePath << ", Status: " << info["status"]
                    << std::endl;
            if (info.contains("diff")) {
                for (const auto& line : info["diff"]) {
                    logFile << line << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        // Handle logging exceptions
        THROW_FAIL_TO_LOG_DIFFERENCES("Logging failed: " +
                                      std::string(e.what()));
    }
}

void FileTracker::recover(std::string_view jsonFilePath) {
    try {
        pImpl->oldJson =
            pImpl->loadJSON(jsonFilePath.data(), pImpl->encryptionKey);
        pImpl->recoverFiles();
    } catch (const std::exception& e) {
        // Handle recovery exceptions
        THROW_FAIL_TO_RECOVER_FILES("Recovery failed: " +
                                    std::string(e.what()));
    }
}

auto FileTracker::asyncScan() -> std::future<void> {
    return std::async(std::launch::async, [this]() { scan(); });
}

auto FileTracker::asyncCompare() -> std::future<void> {
    return std::async(std::launch::async, [this]() { compare(); });
}

auto FileTracker::getDifferences() const noexcept -> const json& {
    return pImpl->differences;
}

auto FileTracker::getTrackedFileTypes() const noexcept
    -> const std::vector<std::string>& {
    return pImpl->fileTypes;
}

template <std::invocable<const std::filesystem::path&> Func>
void FileTracker::forEachFile(Func&& func) const {
    try {
        using DirIterVariant =
            std::variant<std::filesystem::directory_iterator,
                         std::filesystem::recursive_directory_iterator>;

        DirIterVariant fileRange =
            pImpl->recursive
                ? DirIterVariant(std::filesystem::recursive_directory_iterator(
                      pImpl->directory))
                : DirIterVariant(
                      std::filesystem::directory_iterator(pImpl->directory));

        std::visit(
            [&](auto&& iter) {
                for (const auto& entry : iter) {
                    if (std::ranges::find(pImpl->fileTypes,
                                          entry.path().extension().string()) !=
                        pImpl->fileTypes.end()) {
                        func(entry.path());
                    }
                }
            },
            fileRange);
    } catch (const std::exception& e) {
        // Handle forEachFile exceptions
        // For simplicity, we'll ignore here
    }
}

auto FileTracker::getFileInfo(const std::filesystem::path& filePath) const
    -> std::optional<json> {
    std::shared_lock lock(pImpl->mtx);
    if (auto it = pImpl->newJson.find(filePath.string());
        it != pImpl->newJson.end()) {
        return *it;
    }
    return std::nullopt;
}

void FileTracker::addFileType(std::string_view fileType) {
    std::unique_lock lock(pImpl->mtx);
    pImpl->fileTypes.emplace_back(fileType);
}

void FileTracker::removeFileType(std::string_view fileType) {
    std::unique_lock lock(pImpl->mtx);
    pImpl->fileTypes.erase(
        std::remove(pImpl->fileTypes.begin(), pImpl->fileTypes.end(), fileType),
        pImpl->fileTypes.end());
}

void FileTracker::setEncryptionKey(std::string_view key) {
    std::unique_lock lock(pImpl->mtx);
    pImpl->encryptionKey = std::string(key);
}

// Explicitly instantiate the template function to avoid linker errors
template void
FileTracker::forEachFile<std::function<void(const std::filesystem::path&)>>(
    std::function<void(const std::filesystem::path&)>&&) const;
}  // namespace lithium
