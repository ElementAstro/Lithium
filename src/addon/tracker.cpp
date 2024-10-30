#include "tracker.hpp"

#include <algorithm>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <variant>

#include "atom/error/exception.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/aes.hpp"
#include "atom/utils/time.hpp"

struct FileTracker::Impl {
    std::string directory;
    std::string jsonFilePath;
    bool recursive;
    std::vector<std::string> fileTypes;
    json newJson;
    json oldJson;
    json differences;
    std::mutex mtx;
    std::optional<std::string> encryptionKey;

    Impl(std::string_view dir, std::string_view jFilePath,
         std::span<const std::string> types, bool rec)
        : directory(dir),
          jsonFilePath(jFilePath),
          recursive(rec),
          fileTypes(types.begin(), types.end()) {}

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
        using DirIterVariant = std::variant<fs::directory_iterator,
                                            fs::recursive_directory_iterator>;

        DirIterVariant fileRange =
            recursive
                ? DirIterVariant(fs::recursive_directory_iterator(directory))
                : DirIterVariant(fs::directory_iterator(directory));

        std::visit(
            [&](auto&& iter) {
                for (const auto& entry : iter) {
                    if (std::ranges::find(fileTypes,
                                          entry.path().extension().string()) !=
                        fileTypes.end()) {
                        processFile(entry.path());
                    }
                }
            },
            fileRange);

        saveJSON(newJson, jsonFilePath, encryptionKey);
    }

    void processFile(const fs::path& entry) {
        std::string hash = atom::utils::calculateSha256(entry.string());
        std::string lastWriteTime = atom::utils::getChinaTimestampString();

        std::lock_guard lock(mtx);
        newJson[entry.string()] = {{"last_write_time", lastWriteTime},
                                   {"hash", hash},
                                   {"size", fs::file_size(entry)},
                                   {"type", entry.extension().string()}};
    }

    auto compareJSON() -> json {
        json diff;
        for (const auto& [filePath, newFileInfo] : newJson.items()) {
            if (oldJson.contains(filePath)) {
                if (oldJson[filePath]["hash"] != newFileInfo["hash"]) {
                    diff[filePath] = {{"status", "modified"},
                                      {"new", newFileInfo},
                                      {"old", oldJson[filePath]}};
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
            if (!fs::exists(filePath)) {
                std::ofstream outFile(filePath);
                if (outFile.is_open()) {
                    outFile << "This file was recovered based on version: "
                            << fileInfo["last_write_time"] << std::endl;
                    outFile.close();
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
    if (fs::exists(pImpl->jsonFilePath)) {
        pImpl->oldJson =
            pImpl->loadJSON(pImpl->jsonFilePath, pImpl->encryptionKey);
    }
    pImpl->generateJSON();
}

void FileTracker::compare() { pImpl->differences = pImpl->compareJSON(); }

void FileTracker::logDifferences(std::string_view logFilePath) const {
    std::ofstream logFile(logFilePath.data(), std::ios_base::app);
    if (!logFile.is_open()) {
        THROW_FAIL_TO_OPEN_FILE("Failed to open log file: " +
                                 std::string(logFilePath));
    }
    for (const auto& [filePath, info] : pImpl->differences.items()) {
        logFile << "File: " << filePath << ", Status: " << info["status"]
                << std::endl;
    }
}

void FileTracker::recover(std::string_view jsonFilePath) {
    pImpl->oldJson = pImpl->loadJSON(jsonFilePath.data(), pImpl->encryptionKey);
    pImpl->recoverFiles();
}

auto FileTracker::asyncScan() -> std::future<void> {
    return std::async(std::launch::async, [this] { scan(); });
}

auto FileTracker::asyncCompare() -> std::future<void> {
    return std::async(std::launch::async, [this] { compare(); });
}

auto FileTracker::getDifferences() const noexcept -> const json& {
    return pImpl->differences;
}

auto FileTracker::getTrackedFileTypes() const noexcept
    -> const std::vector<std::string>& {
    return pImpl->fileTypes;
}

template <std::invocable<const fs::path&> Func>
void FileTracker::forEachFile(Func&& func) const {
    using DirIterVariant =
        std::variant<fs::directory_iterator, fs::recursive_directory_iterator>;

    DirIterVariant fileRange =
        pImpl->recursive
            ? DirIterVariant(fs::recursive_directory_iterator(pImpl->directory))
            : DirIterVariant(fs::directory_iterator(pImpl->directory));

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
}

auto FileTracker::getFileInfo(const fs::path& filePath) const
    -> std::optional<json> {
    if (auto it = pImpl->newJson.find(filePath.string());
        it != pImpl->newJson.end()) {
        return *it;
    }
    return std::nullopt;
}

void FileTracker::addFileType(std::string_view fileType) {
    pImpl->fileTypes.emplace_back(fileType);
}

void FileTracker::removeFileType(std::string_view fileType) {
    pImpl->fileTypes.erase(
        std::remove(pImpl->fileTypes.begin(), pImpl->fileTypes.end(), fileType),
        pImpl->fileTypes.end());
}

void FileTracker::setEncryptionKey(std::string_view key) {
    pImpl->encryptionKey = std::string(key);
}

// Explicitly instantiate the template function to avoid linker errors
template void FileTracker::forEachFile<std::function<void(const fs::path&)>>(
    std::function<void(const fs::path&)>&&) const;