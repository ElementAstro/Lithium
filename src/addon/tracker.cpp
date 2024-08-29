#include "tracker.hpp"

#include <openssl/evp.h>
#include <openssl/sha.h>

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <thread>
#include <utility>

#include "atom/type/json.hpp"
#include "atom/utils/aes.hpp"
namespace fs = std::filesystem;

// Implementation of the FileTracker class
struct FileTracker::Impl {
    std::string directory;
    std::string jsonFilePath;
    bool recursive;
    std::vector<std::string> fileTypes;
    json newJson;
    json oldJson;
    json differences;
    std::mutex mtx;

    // Constructor
    Impl(std::string dir, std::string jFilePath,
         const std::vector<std::string>& types, bool rec)
        : directory(std::move(dir)),
          jsonFilePath(std::move(jFilePath)),
          recursive(rec),
          fileTypes(types) {}

    // Get last write time
    static auto getLastWriteTime(const fs::path& filePath) -> std::string;

    // Save JSON to a file
    static void saveJSON(const json& j, const std::string& filePath);

    // Load JSON from a file
    static auto loadJSON(const std::string& filePath) -> json;

    // Generate the JSON data for files in the specified directory
    void generateJSON();

    // Compare the old and new JSON data
    auto compareJSON() -> json;

    // Recover files from the JSON state
    void recoverFiles();
};

// Get the last write time of a file
auto FileTracker::Impl::getLastWriteTime(const fs::path& filePath)
    -> std::string {
    auto ftime = fs::last_write_time(filePath);
    auto sctp =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - fs::file_time_type::clock::now() +
            std::chrono::system_clock::now());
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    return std::asctime(std::localtime(&cftime));
}

// Save JSON object to a file
void FileTracker::Impl::saveJSON(const json& j, const std::string& filePath) {
    std::ofstream outFile(filePath);
    outFile << std::setw(4) << j << std::endl;
}

// Load JSON object from a file
auto FileTracker::Impl::loadJSON(const std::string& filePath) -> json {
    std::ifstream inFile(filePath);
    if (!inFile.is_open()) {
        return json();
    }
    json j;
    inFile >> j;
    return j;
}

// Generate JSON for all tracked files in the specified directory
void FileTracker::Impl::generateJSON() {
    std::vector<std::thread> threads;

    auto process = [&](const fs::path& entry) {
        std::string hash = atom::utils::calculateSha256(entry.string());
        std::string lastWriteTime = getLastWriteTime(entry);
        std::lock_guard lock(mtx);

        newJson[entry.string()] = {{"last_write_time", lastWriteTime},
                                   {"hash", hash},
                                   {"size", fs::file_size(entry)},
                                   {"type", entry.extension().string()}};
    };

    if (recursive) {
        for (const auto& entry : fs::recursive_directory_iterator(directory)) {
            if (std::find(fileTypes.begin(), fileTypes.end(),
                          entry.path().extension().string()) !=
                fileTypes.end()) {
                threads.emplace_back(process, entry.path());
            }
        }
    } else {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (std::find(fileTypes.begin(), fileTypes.end(),
                          entry.path().extension().string()) !=
                fileTypes.end()) {
                threads.emplace_back(process, entry.path());
            }
        }
    }

    for (auto& th : threads) {
        if (th.joinable()) {
            th.join();
        }
    }

    saveJSON(newJson, jsonFilePath);
}

// Compare new and old JSON objects for differences
auto FileTracker::Impl::compareJSON() -> json {
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

// Recover files from the JSON state
void FileTracker::Impl::recoverFiles() {
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

// Constructor
FileTracker::FileTracker(const std::string& directory,
                         const std::string& jsonFilePath,
                         const std::vector<std::string>& fileTypes,
                         bool recursive)
    : pImpl(std::make_unique<Impl>(directory, jsonFilePath, fileTypes,
                                   recursive)) {}

// Destructor
FileTracker::~FileTracker() = default;

// Scan for files and generate JSON
void FileTracker::scan() {
    if (fs::exists(pImpl->jsonFilePath)) {
        pImpl->oldJson = pImpl->loadJSON(pImpl->jsonFilePath);
    }
    pImpl->generateJSON();
}

// Compare the loaded JSON data
void FileTracker::compare() { pImpl->differences = pImpl->compareJSON(); }

// Log the differences found to a log file
void FileTracker::logDifferences(const std::string& logFilePath) {
    std::ofstream logFile(logFilePath, std::ios_base::app);
    if (logFile.is_open()) {
        for (const auto& [filePath, info] : pImpl->differences.items()) {
            logFile << "File: " << filePath << ", Status: " << info["status"]
                    << std::endl;
        }
    }
}

// Recover the files based on the saved JSON data
void FileTracker::recover(const std::string& jsonFilePath) {
    pImpl->oldJson = pImpl->loadJSON(jsonFilePath);
    pImpl->recoverFiles();
}

// Get the differences in JSON format
auto FileTracker::getDifferences() const -> const json& {
    return pImpl->differences;
}

// Get the list of tracked file types
auto FileTracker::getTrackedFileTypes() const
    -> const std::vector<std::string>& {
    return pImpl->fileTypes;
}
