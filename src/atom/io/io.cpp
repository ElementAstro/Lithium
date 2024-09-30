/*
 * io.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-3

Description: IO

**************************************************/

#include "io.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <regex>
#include <string_view>
#include <thread>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/string.hpp"
#include "error/exception.hpp"

#ifdef __linux
#include <dirent.h>
#include <sys/stat.h>
#endif

namespace fs = std::filesystem;
using json = nlohmann::json;

#ifdef _WIN32
#include <windows.h>
const std::regex FOLDER_NAME_REGEX(R"(^[^\/?*:;{}\\]+[^\\]*$)");
const std::regex FILE_NAME_REGEX("^[^\\/:*?\"<>|]+$");
#else
const std::regex FOLDER_NAME_REGEX("^[^/]+$");
const std::regex FILE_NAME_REGEX("^[^/]+$");
#endif

#ifdef _MSC_VER
#undef min
#endif

#define ATOM_IO_CHECK_ARGUMENT(value)                              \
    if ((value).empty()) {                                         \
        LOG_F(ERROR, "{}: Invalid argument: {}", __func__, value); \
        return false;                                              \
    }

#define ATOM_IO_CHECK_ARGUMENT_S(value)                            \
    if ((value).empty()) {                                         \
        LOG_F(ERROR, "{}: Invalid argument: {}", __func__, value); \
        return "";                                                 \
    }

namespace atom::io {

auto createDirectory(const std::string &path) -> bool {
    ATOM_IO_CHECK_ARGUMENT(path);
    try {
        return fs::create_directory(path);
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to create directory {}: {}", path, e.what());
        return false;
    }
}

auto createDirectoriesRecursive(
    const fs::path &basePath, const std::vector<std::string> &subdirs,
    const CreateDirectoriesOptions &options = {}) -> bool {
    for (const auto &subdir : subdirs | std::views::filter([&](const auto &s) {
                                  return options.filter(s);
                              })) {
        auto fullPath = basePath / subdir;
        if (fs::exists(fullPath) && fs::is_directory(fullPath)) {
            if (options.verbose) {
                LOG_F(INFO, "Directory already exists: {}", fullPath.string());
            }
            continue;
        }

        if (!options.dryRun && !fs::create_directories(fullPath)) {
            LOG_F(ERROR, "Failed to create directory {}", fullPath.string());
            return false;
        }

        if (options.verbose) {
            LOG_F(INFO, "Created directory: {}", fullPath.string());
        }
        options.onCreate(fullPath.string());
        if (options.delay > 0) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(options.delay));
        }
    }
    return true;
}

auto removeDirectory(const std::string &path) -> bool {
    ATOM_IO_CHECK_ARGUMENT(path);
    try {
        fs::remove_all(path);
        DLOG_F(INFO, "Directory removed: {}", path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to remove directory {}: {}", path, e.what());
        return false;
    }
}

auto removeDirectoriesRecursive(
    const fs::path &basePath, const std::vector<std::string> &subdirs,
    const CreateDirectoriesOptions &options) -> bool {
    for (const auto &subdir : subdirs | std::views::filter([&](const auto &s) {
                                  return options.filter(s);
                              })) {
        auto fullPath = basePath / subdir;
        if (!fs::exists(fullPath)) {
            if (options.verbose) {
                LOG_F(INFO, "Directory does not exist: {}", fullPath.string());
            }
            continue;
        }

        try {
            fs::remove_all(fullPath);
            if (options.verbose) {
                LOG_F(INFO, "Deleted directory: {}", fullPath.string());
            }
        } catch (const fs::filesystem_error &e) {
            LOG_F(ERROR, "Failed to delete directory {}: {}", fullPath.string(),
                  e.what());
            return false;
        }

        options.onDelete(fullPath.string());
        if (options.delay > 0) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(options.delay));
        }
    }
    return true;
}

auto renameDirectory(const std::string &old_path,
                     const std::string &new_path) -> bool {
    ATOM_IO_CHECK_ARGUMENT(old_path);
    ATOM_IO_CHECK_ARGUMENT(new_path);
    return moveDirectory(old_path, new_path);
}

auto moveDirectory(const std::string &old_path,
                   const std::string &new_path) -> bool {
    ATOM_IO_CHECK_ARGUMENT(old_path);
    ATOM_IO_CHECK_ARGUMENT(new_path);
    try {
        fs::rename(old_path, new_path);
        DLOG_F(INFO, "Directory moved from {} to {}", old_path, new_path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to move directory from {} to {}: {}", old_path,
              new_path, e.what());
        return false;
    }
}

auto copyFile(const std::string &src_path,
              const std::string &dst_path) -> bool {
    ATOM_IO_CHECK_ARGUMENT(src_path);
    ATOM_IO_CHECK_ARGUMENT(dst_path);
    try {
        fs::copy_file(src_path, dst_path);
        DLOG_F(INFO, "File copied from {} to {}", src_path, dst_path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to copy file from {} to {}: {}", src_path,
              dst_path, e.what());
        return false;
    }
}

auto moveFile(const std::string &src_path,
              const std::string &dst_path) -> bool {
    return renameFile(src_path, dst_path);
}

auto renameFile(const std::string &old_path,
                const std::string &new_path) -> bool {
    ATOM_IO_CHECK_ARGUMENT(old_path);
    ATOM_IO_CHECK_ARGUMENT(new_path);
    try {
        fs::rename(old_path, new_path);
        DLOG_F(INFO, "File renamed from {} to {}", old_path, new_path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to rename file from {} to {}: {}", old_path,
              new_path, e.what());
        return false;
    }
}

auto removeFile(const std::string &path) -> bool {
    ATOM_IO_CHECK_ARGUMENT(path);
    try {
        fs::remove(path);
        DLOG_F(INFO, "File removed: {}", path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to remove file {}: {}", path, e.what());
        return false;
    }
}

auto createSymlink(const std::string &target_path,
                   const std::string &symlink_path) -> bool {
    ATOM_IO_CHECK_ARGUMENT(target_path);
    ATOM_IO_CHECK_ARGUMENT(symlink_path);
    try {
        fs::create_symlink(target_path, symlink_path);
        DLOG_F(INFO, "Symlink created from {} to {}", target_path,
               symlink_path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to create symlink from {} to {}: {}", target_path,
              symlink_path, e.what());
        return false;
    }
}

auto removeSymlink(const std::string &path) -> bool { return removeFile(path); }

auto fileSize(const std::string &path) -> std::uintmax_t {
    try {
        return fs::file_size(path);
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to get file size of {}: {}", path, e.what());
        return 0;
    }
}

auto truncateFile(const std::string &path, std::streamsize size) -> bool {
    std::ofstream file(path,
                       std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }
    file.seekp(size);
    file.put('\0');
    return true;
}

auto convertToLinuxPath(std::string_view windows_path) -> std::string {
    std::string linuxPath(windows_path);
    std::ranges::replace(linuxPath, '\\', '/');
    if (linuxPath.length() >= 2 && linuxPath[1] == ':') {
        linuxPath[0] = std::tolower(linuxPath[0]);
    }
    return linuxPath;
}

auto convertToWindowsPath(std::string_view linux_path) -> std::string {
    std::string windowsPath(linux_path);
    std::ranges::replace(windowsPath, '/', '\\');
    if (windowsPath.length() >= 2 && std::islower(windowsPath[0]) &&
        windowsPath[1] == ':') {
        windowsPath[0] = std::toupper(windowsPath[0]);
    }
    return windowsPath;
}

auto normalizePath(std::string_view path) -> std::string {
    std::string normalizedPath(path);
    char preferredSeparator = fs::path::preferred_separator;
    std::ranges::replace(normalizedPath, '/', preferredSeparator);
    std::ranges::replace(normalizedPath, '\\', preferredSeparator);
    return normalizedPath;
}

auto normPath(std::string_view raw_path) -> std::string {
    std::string path = normalizePath(raw_path);
    fs::path fsPath(path);
    fs::path normalizedFsPath;

    for (const auto &part : fsPath) {
        if (part == ".") {
            continue;
        }
        if (part == "..") {
            if (!normalizedFsPath.empty() &&
                normalizedFsPath.filename() != "..") {
                normalizedFsPath = normalizedFsPath.parent_path();
            } else {
                normalizedFsPath /= part;
            }
        } else {
            normalizedFsPath /= part;
        }
    }
    return normalizedFsPath.string().empty() ? "/" : normalizedFsPath.string();
}

void walk(const fs::path &root, bool recursive,
          const std::function<void(const fs::path &)> &callback) {
    for (const auto &entry : fs::directory_iterator(root)) {
        callback(entry.path());
        if (recursive && fs::is_directory(entry)) {
            walk(entry.path(), recursive, callback);
        }
    }
}

auto buildJsonStructure(const fs::path &root, bool recursive) -> json {
    json folder = {{"path", root.generic_string()},
                   {"directories", json::array()},
                   {"files", json::array()}};
    walk(root, recursive, [&](const fs::path &entry) {
        if (fs::is_directory(entry)) {
            folder["directories"].push_back(
                buildJsonStructure(entry, recursive));
        } else {
            folder["files"].push_back(entry.generic_string());
        }
    });
    return folder;
}

auto jwalk(const std::string &root) -> std::string {
    fs::path rootPath(root);
    if (!isFolderExists(rootPath.string())) {
        return "";
    }
    return buildJsonStructure(rootPath, true).dump();
}

void fwalk(const fs::path &root,
           const std::function<void(const fs::path &)> &callback) {
    walk(root, true, callback);
}

auto isFolderNameValid(const std::string &folderName) -> bool {
    ATOM_IO_CHECK_ARGUMENT(folderName);
    return std::regex_match(folderName, FOLDER_NAME_REGEX);
}

auto isFileNameValid(const std::string &fileName) -> bool {
    ATOM_IO_CHECK_ARGUMENT(fileName);
    return std::regex_match(fileName, FILE_NAME_REGEX);
}

auto isFolderExists(const std::string &folderName) -> bool {
    return isFolderNameValid(folderName) && fs::exists(folderName) &&
           fs::is_directory(folderName);
}

auto isFileExists(const std::string &fileName) -> bool {
    return isFileNameValid(fileName) && fs::exists(fileName) &&
           fs::is_regular_file(fileName);
}

auto isFolderEmpty(const std::string &folderName) -> bool {
    if (!isFolderExists(folderName))
        return false;
    return fs::is_empty(folderName);
}

auto isAbsolutePath(const std::string &path) -> bool {
    return fs::path(path).is_absolute();
}

auto changeWorkingDirectory(const std::string &directoryPath) -> bool {
    if (!isFolderExists(directoryPath)) {
        LOG_F(ERROR, "Directory does not exist: {}", directoryPath);
        return false;
    }
    try {
        fs::current_path(directoryPath);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to change working directory: {}", e.what());
        return false;
    }
}

auto getFileTimes(const std::string &filePath)
    -> std::pair<std::string, std::string> {
    std::pair<std::string, std::string> fileTimes;

#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (GetFileAttributesExW(atom::utils::stringToWString(filePath).c_str(),
                             GetFileExInfoStandard, &fileInfo) == 0) {
        LOG_F(ERROR, "Error getting file information.");
        return fileTimes;
    }

    FILETIME createTime, modifyTime;
    FileTimeToLocalFileTime(&fileInfo.ftCreationTime, &createTime);
    FileTimeToLocalFileTime(&fileInfo.ftLastWriteTime, &modifyTime);

    SYSTEMTIME createSysTime, modifySysTime;
    FileTimeToSystemTime(&createTime, &createSysTime);
    FileTimeToSystemTime(&modifyTime, &modifySysTime);

    char createTimeStr[20], modifyTimeStr[20];
    std::snprintf(createTimeStr, sizeof(createTimeStr),
                  "%04d/%02d/%02d %02d:%02d:%02d", createSysTime.wYear,
                  createSysTime.wMonth, createSysTime.wDay, createSysTime.wHour,
                  createSysTime.wMinute, createSysTime.wSecond);
    std::snprintf(modifyTimeStr, sizeof(modifyTimeStr),
                  "%04d/%02d/%02d %02d:%02d:%02d", modifySysTime.wYear,
                  modifySysTime.wMonth, modifySysTime.wDay, modifySysTime.wHour,
                  modifySysTime.wMinute, modifySysTime.wSecond);

    fileTimes.first = createTimeStr;
    fileTimes.second = modifyTimeStr;

#else
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) != 0) {
        LOG_F(ERROR, "Error getting file information.");
        return fileTimes;
    }

    char createTimeStr[20], modifyTimeStr[20];
    auto *createTimeTm = localtime(&fileInfo.st_ctime);
    auto *modifyTimeTm = localtime(&fileInfo.st_mtime);

    strftime(createTimeStr, sizeof(createTimeStr), "%Y/%m/%d %H:%M:%S",
             createTimeTm);
    strftime(modifyTimeStr, sizeof(modifyTimeStr), "%Y/%m/%d %H:%M:%S",
             modifyTimeTm);

    fileTimes.first = createTimeStr;
    fileTimes.second = modifyTimeStr;

#endif
    return fileTimes;
}

auto checkFileTypeInFolder(const std::string &folderPath,
                           const std::string &fileType,
                           FileOption fileOption) -> std::vector<std::string> {
    std::vector<std::string> files;
    try {
        for (const auto &entry : fs::directory_iterator(folderPath)) {
            if (entry.is_regular_file() &&
                entry.path().extension() == fileType) {
                files.push_back(fileOption == FileOption::PATH
                                    ? entry.path().string()
                                    : entry.path().filename().string());
            }
        }
    } catch (const fs::filesystem_error &ex) {
        LOG_F(ERROR, "Failed to check files in folder: {}", ex.what());
    }
    return files;
}

auto isExecutableFile(const std::string &fileName,
                      const std::string &fileExt = "") -> bool {
#ifdef _WIN32
    fs::path filePath = fileName + fileExt;
#else
    fs::path filePath = fileName;
#endif

    DLOG_F(INFO, "Checking file '{}'.", filePath.string());
    if (!fs::exists(filePath) || !fs::is_regular_file(filePath)) {
        DLOG_F(WARNING,
               "The file '{}' is not a regular file or does not exist.",
               filePath.string());
        return false;
    }

#ifndef _WIN32
    if ((fs::status(filePath).permissions() & fs::perms::owner_exec) ==
        fs::perms::none) {
        DLOG_F(WARNING, "The file '{}' is not executable.", filePath.string());
        return false;
    }
#endif

    DLOG_F(INFO, "The file '{}' exists and is executable.", filePath.string());
    return true;
}

auto getFileSize(const std::string &filePath) -> std::size_t {
    return fs::file_size(filePath);
}

auto calculateChunkSize(std::size_t fileSize, int numChunks) -> std::size_t {
    return (fileSize + numChunks - 1) / numChunks;
}

void splitFile(const std::string &filePath, std::size_t chunkSize,
               const std::string &outputPattern) {
    std::ifstream inputFile(filePath, std::ios::binary);
    if (!inputFile) {
        LOG_F(ERROR, "Failed to open file: {}", filePath);
        return;
    }

    char *buffer = new char[chunkSize];
    std::size_t fileSize = getFileSize(filePath);
    int partNumber = 0;

    while (fileSize > 0) {
        std::ostringstream partFileName;
        partFileName << (outputPattern.empty() ? filePath : outputPattern)
                     << ".part" << partNumber;

        std::ofstream outputFile(partFileName.str(), std::ios::binary);
        if (!outputFile) {
            LOG_F(ERROR, "Failed to create part file: {}", partFileName.str());
            delete[] buffer;
            return;
        }

        std::size_t bytesToRead = std::min(chunkSize, fileSize);
        inputFile.read(buffer, bytesToRead);
        outputFile.write(buffer, bytesToRead);

        fileSize -= bytesToRead;
        ++partNumber;
    }

    delete[] buffer;
    LOG_F(INFO, "File split into {} parts.", partNumber);
}

void mergeFiles(const std::string &outputFilePath,
                const std::vector<std::string> &partFiles) {
    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile) {
        LOG_F(ERROR, "Failed to create output file: {}", outputFilePath);
        return;
    }

    char buffer[1024];
    for (const auto &partFile : partFiles) {
        std::ifstream inputFile(partFile, std::ios::binary);
        if (!inputFile) {
            LOG_F(ERROR, "Failed to open part file: {}", partFile);
            return;
        }

        while (inputFile.read(buffer, sizeof(buffer))) {
            outputFile.write(buffer, sizeof(buffer));
        }
        outputFile.write(buffer, inputFile.gcount());

        inputFile.close();
    }

    outputFile.close();
    LOG_F(INFO, "Files merged into {}", outputFilePath);
}

void quickSplit(const std::string &filePath, int numChunks,
                const std::string &outputPattern) {
    std::size_t fileSize = getFileSize(filePath);
    std::size_t chunkSize = calculateChunkSize(fileSize, numChunks);
    splitFile(filePath, chunkSize, outputPattern);
}

void quickMerge(const std::string &outputFilePath,
                const std::string &partPattern, int numChunks) {
    std::vector<std::string> partFiles;
    for (int i = 0; i < numChunks; ++i) {
        std::ostringstream partFileName;
        partFileName << partPattern << i;
        partFiles.push_back(partFileName.str());
    }
    mergeFiles(outputFilePath, partFiles);
}

#ifdef _WIN32
const char PATH_SEPARATORS[] = "/\\";
#else
const char PATH_SEPARATORS[] = "/";
#endif

auto getExecutableNameFromPath(const std::string &path) -> std::string {
    if (path.empty()) {
        THROW_INVALID_ARGUMENT("The provided path is empty.");
    }

    size_t lastSlashPos = path.find_last_of(PATH_SEPARATORS);

    if (lastSlashPos == std::string::npos) {
        if (path.find('.') == std::string::npos) {
            THROW_INVALID_ARGUMENT(
                "The provided path does not contain a valid file name.");
        }
        return path;
    }

    std::string fileName = path.substr(lastSlashPos + 1);

    if (fileName.empty()) {
        THROW_INVALID_ARGUMENT(
            "The provided path ends with a slash and contains no file name.");
    }

    size_t dotPos = fileName.find_last_of('.');
    if (dotPos == std::string::npos) {
        THROW_INVALID_ARGUMENT("The file name does not contain an extension.");
    }

    return fileName;
}

}  // namespace atom::io
