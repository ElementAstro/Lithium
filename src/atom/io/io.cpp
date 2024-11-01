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
    LOG_F(INFO, "createDirectory called with path: {}", path);
    ATOM_IO_CHECK_ARGUMENT(path);
    try {
        bool result = fs::create_directory(path);
        LOG_F(INFO, "Directory created: {}", path);
        return result;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to create directory {}: {}", path, e.what());
        return false;
    }
}

auto createDirectoriesRecursive(
    const fs::path &basePath, const std::vector<std::string> &subdirs,
    const CreateDirectoriesOptions &options = {}) -> bool {
    LOG_F(INFO, "createDirectoriesRecursive called with basePath: {}",
          basePath.string());
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
    LOG_F(INFO, "createDirectoriesRecursive completed");
    return true;
}

auto removeDirectory(const std::string &path) -> bool {
    LOG_F(INFO, "removeDirectory called with path: {}", path);
    ATOM_IO_CHECK_ARGUMENT(path);
    try {
        fs::remove_all(path);
        LOG_F(INFO, "Directory removed: {}", path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to remove directory {}: {}", path, e.what());
        return false;
    }
}

auto removeDirectoriesRecursive(
    const fs::path &basePath, const std::vector<std::string> &subdirs,
    const CreateDirectoriesOptions &options) -> bool {
    LOG_F(INFO, "removeDirectoriesRecursive called with basePath: {}",
          basePath.string());
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
    LOG_F(INFO, "removeDirectoriesRecursive completed");
    return true;
}

auto renameDirectory(const std::string &old_path,
                     const std::string &new_path) -> bool {
    LOG_F(INFO, "renameDirectory called with old_path: {}, new_path: {}",
          old_path, new_path);
    ATOM_IO_CHECK_ARGUMENT(old_path);
    ATOM_IO_CHECK_ARGUMENT(new_path);
    return moveDirectory(old_path, new_path);
}

auto moveDirectory(const std::string &old_path,
                   const std::string &new_path) -> bool {
    LOG_F(INFO, "moveDirectory called with old_path: {}, new_path: {}",
          old_path, new_path);
    ATOM_IO_CHECK_ARGUMENT(old_path);
    ATOM_IO_CHECK_ARGUMENT(new_path);
    try {
        fs::rename(old_path, new_path);
        LOG_F(INFO, "Directory moved from {} to {}", old_path, new_path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to move directory from {} to {}: {}", old_path,
              new_path, e.what());
        return false;
    }
}

auto copyFile(const std::string &src_path,
              const std::string &dst_path) -> bool {
    LOG_F(INFO, "copyFile called with src_path: {}, dst_path: {}", src_path,
          dst_path);
    ATOM_IO_CHECK_ARGUMENT(src_path);
    ATOM_IO_CHECK_ARGUMENT(dst_path);
    try {
        fs::copy_file(src_path, dst_path);
        LOG_F(INFO, "File copied from {} to {}", src_path, dst_path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to copy file from {} to {}: {}", src_path,
              dst_path, e.what());
        return false;
    }
}

auto moveFile(const std::string &src_path,
              const std::string &dst_path) -> bool {
    LOG_F(INFO, "moveFile called with src_path: {}, dst_path: {}", src_path,
          dst_path);
    return renameFile(src_path, dst_path);
}

auto renameFile(const std::string &old_path,
                const std::string &new_path) -> bool {
    LOG_F(INFO, "renameFile called with old_path: {}, new_path: {}", old_path,
          new_path);
    ATOM_IO_CHECK_ARGUMENT(old_path);
    ATOM_IO_CHECK_ARGUMENT(new_path);
    try {
        fs::rename(old_path, new_path);
        LOG_F(INFO, "File renamed from {} to {}", old_path, new_path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to rename file from {} to {}: {}", old_path,
              new_path, e.what());
        return false;
    }
}

auto removeFile(const std::string &path) -> bool {
    LOG_F(INFO, "removeFile called with path: {}", path);
    ATOM_IO_CHECK_ARGUMENT(path);
    try {
        fs::remove(path);
        LOG_F(INFO, "File removed: {}", path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to remove file {}: {}", path, e.what());
        return false;
    }
}

auto createSymlink(const std::string &target_path,
                   const std::string &symlink_path) -> bool {
    LOG_F(INFO, "createSymlink called with target_path: {}, symlink_path: {}",
          target_path, symlink_path);
    ATOM_IO_CHECK_ARGUMENT(target_path);
    ATOM_IO_CHECK_ARGUMENT(symlink_path);
    try {
        fs::create_symlink(target_path, symlink_path);
        LOG_F(INFO, "Symlink created from {} to {}", target_path, symlink_path);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to create symlink from {} to {}: {}", target_path,
              symlink_path, e.what());
        return false;
    }
}

auto removeSymlink(const std::string &path) -> bool {
    LOG_F(INFO, "removeSymlink called with path: {}", path);
    return removeFile(path);
}

auto fileSize(const std::string &path) -> std::uintmax_t {
    LOG_F(INFO, "fileSize called with path: {}", path);
    try {
        std::uintmax_t size = fs::file_size(path);
        LOG_F(INFO, "File size of {}: {}", path, size);
        return size;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to get file size of {}: {}", path, e.what());
        return 0;
    }
}

auto truncateFile(const std::string &path, std::streamsize size) -> bool {
    LOG_F(INFO, "truncateFile called with path: {}, size: {}", path, size);
    std::ofstream file(path,
                       std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        LOG_F(ERROR, "Failed to open file for truncation: {}", path);
        return false;
    }
    file.seekp(size);
    file.put('\0');
    LOG_F(INFO, "File truncated: {}", path);
    return true;
}

auto convertToLinuxPath(std::string_view windows_path) -> std::string {
    LOG_F(INFO, "convertToLinuxPath called with windows_path: {}",
          windows_path);
    std::string linuxPath(windows_path);
    std::ranges::replace(linuxPath, '\\', '/');
    if (linuxPath.length() >= 2 && linuxPath[1] == ':') {
        linuxPath[0] = std::tolower(linuxPath[0]);
    }
    LOG_F(INFO, "Converted to Linux path: {}", linuxPath);
    return linuxPath;
}

auto convertToWindowsPath(std::string_view linux_path) -> std::string {
    LOG_F(INFO, "convertToWindowsPath called with linux_path: {}", linux_path);
    std::string windowsPath(linux_path);
    std::ranges::replace(windowsPath, '/', '\\');
    if (windowsPath.length() >= 2 && std::islower(windowsPath[0]) &&
        windowsPath[1] == ':') {
        windowsPath[0] = std::toupper(windowsPath[0]);
    }
    LOG_F(INFO, "Converted to Windows path: {}", windowsPath);
    return windowsPath;
}

auto normalizePath(std::string_view path) -> std::string {
    LOG_F(INFO, "normalizePath called with path: {}", path);
    std::string normalizedPath(path);
    char preferredSeparator = fs::path::preferred_separator;
    std::ranges::replace(normalizedPath, '/', preferredSeparator);
    std::ranges::replace(normalizedPath, '\\', preferredSeparator);
    LOG_F(INFO, "Normalized path: {}", normalizedPath);
    return normalizedPath;
}

auto normPath(std::string_view raw_path) -> std::string {
    LOG_F(INFO, "normPath called with raw_path: {}", raw_path);
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
    std::string result =
        normalizedFsPath.string().empty() ? "/" : normalizedFsPath.string();
    LOG_F(INFO, "Normalized path: {}", result);
    return result;
}

void walk(const fs::path &root, bool recursive,
          const std::function<void(const fs::path &)> &callback) {
    LOG_F(INFO, "walk called with root: {}, recursive: {}", root.string(),
          recursive);
    for (const auto &entry : fs::directory_iterator(root)) {
        callback(entry.path());
        if (recursive && fs::is_directory(entry)) {
            walk(entry.path(), recursive, callback);
        }
    }
    LOG_F(INFO, "walk completed for root: {}", root.string());
}

auto buildJsonStructure(const fs::path &root, bool recursive) -> json {
    LOG_F(INFO, "buildJsonStructure called with root: {}, recursive: {}",
          root.string(), recursive);
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
    LOG_F(INFO, "buildJsonStructure completed for root: {}", root.string());
    return folder;
}

auto jwalk(const std::string &root) -> std::string {
    LOG_F(INFO, "jwalk called with root: {}", root);
    fs::path rootPath(root);
    if (!isFolderExists(rootPath.string())) {
        LOG_F(WARNING, "Folder does not exist: {}", root);
        return "";
    }
    std::string result = buildJsonStructure(rootPath, true).dump();
    LOG_F(INFO, "jwalk completed for root: {}", root);
    return result;
}

void fwalk(const fs::path &root,
           const std::function<void(const fs::path &)> &callback) {
    LOG_F(INFO, "fwalk called with root: {}", root.string());
    walk(root, true, callback);
    LOG_F(INFO, "fwalk completed for root: {}", root.string());
}

auto isFolderNameValid(const std::string &folderName) -> bool {
    LOG_F(INFO, "isFolderNameValid called with folderName: {}", folderName);
    bool result = std::regex_match(folderName, FOLDER_NAME_REGEX);
    LOG_F(INFO, "isFolderNameValid returning: {}", result);
    return result;
}

auto isFileNameValid(const std::string &fileName) -> bool {
    LOG_F(INFO, "isFileNameValid called with fileName: {}", fileName);
    bool result = std::regex_match(fileName, FILE_NAME_REGEX);
    LOG_F(INFO, "isFileNameValid returning: {}", result);
    return result;
}

auto isFolderExists(const std::string &folderName) -> bool {
    LOG_F(INFO, "isFolderExists called with folderName: {}", folderName);
    bool result = isFolderNameValid(folderName) && fs::exists(folderName) &&
                  fs::is_directory(folderName);
    LOG_F(INFO, "isFolderExists returning: {}", result);
    return result;
}

auto isFileExists(const std::string &fileName) -> bool {
    LOG_F(INFO, "isFileExists called with fileName: {}", fileName);
    bool result = isFileNameValid(fileName) && fs::exists(fileName) &&
                  fs::is_regular_file(fileName);
    LOG_F(INFO, "isFileExists returning: {}", result);
    return result;
}

auto isFolderEmpty(const std::string &folderName) -> bool {
    LOG_F(INFO, "isFolderEmpty called with folderName: {}", folderName);
    if (!isFolderExists(folderName)) {
        LOG_F(WARNING, "Folder does not exist: {}", folderName);
        return false;
    }
    bool result = fs::is_empty(folderName);
    LOG_F(INFO, "isFolderEmpty returning: {}", result);
    return result;
}

auto isAbsolutePath(const std::string &path) -> bool {
    LOG_F(INFO, "isAbsolutePath called with path: {}", path);
    bool result = fs::path(path).is_absolute();
    LOG_F(INFO, "isAbsolutePath returning: {}", result);
    return result;
}

auto changeWorkingDirectory(const std::string &directoryPath) -> bool {
    LOG_F(INFO, "changeWorkingDirectory called with directoryPath: {}",
          directoryPath);
    if (!isFolderExists(directoryPath)) {
        LOG_F(ERROR, "Directory does not exist: {}", directoryPath);
        return false;
    }
    try {
        fs::current_path(directoryPath);
        LOG_F(INFO, "Changed working directory to: {}", directoryPath);
        return true;
    } catch (const fs::filesystem_error &e) {
        LOG_F(ERROR, "Failed to change working directory: {}", e.what());
        return false;
    }
}

auto getFileTimes(const std::string &filePath)
    -> std::pair<std::string, std::string> {
    LOG_F(INFO, "getFileTimes called with filePath: {}", filePath);
    std::pair<std::string, std::string> fileTimes;

#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (GetFileAttributesExW(atom::utils::stringToWString(filePath).c_str(),
                             GetFileExInfoStandard, &fileInfo) == 0) {
        LOG_F(ERROR, "Error getting file information.");
        return fileTimes;
    }

    FILETIME createTime;
    FILETIME modifyTime;
    FileTimeToLocalFileTime(&fileInfo.ftCreationTime, &createTime);
    FileTimeToLocalFileTime(&fileInfo.ftLastWriteTime, &modifyTime);

    SYSTEMTIME createSysTime;
    SYSTEMTIME modifySysTime;
    FileTimeToSystemTime(&createTime, &createSysTime);
    FileTimeToSystemTime(&modifyTime, &modifySysTime);

    std::ostringstream createTimeStream;
    createTimeStream << std::setw(4) << createSysTime.wYear << "/"
                     << std::setw(2) << std::setfill('0')
                     << createSysTime.wMonth << "/" << std::setw(2)
                     << std::setfill('0') << createSysTime.wDay << " "
                     << std::setw(2) << std::setfill('0') << createSysTime.wHour
                     << ":" << std::setw(2) << std::setfill('0')
                     << createSysTime.wMinute << ":" << std::setw(2)
                     << std::setfill('0') << createSysTime.wSecond;

    std::ostringstream modifyTimeStream;
    modifyTimeStream << std::setw(4) << modifySysTime.wYear << "/"
                     << std::setw(2) << std::setfill('0')
                     << modifySysTime.wMonth << "/" << std::setw(2)
                     << std::setfill('0') << modifySysTime.wDay << " "
                     << std::setw(2) << std::setfill('0') << modifySysTime.wHour
                     << ":" << std::setw(2) << std::setfill('0')
                     << modifySysTime.wMinute << ":" << std::setw(2)
                     << std::setfill('0') << modifySysTime.wSecond;

    fileTimes.first = createTimeStream.str();
    fileTimes.second = modifyTimeStream.str();

#else
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) != 0) {
        LOG_F(ERROR, "Error getting file information.");
        return fileTimes;
    }

    std::array<char, ATOM_PTR_SIZE> createTimeStr;
    std::array<char, ATOM_PTR_SIZE> modifyTimeStr;
    auto *createTimeTm = localtime(&fileInfo.st_ctime);
    auto *modifyTimeTm = localtime(&fileInfo.st_mtime);

    strftime(createTimeStr.data(), createTimeStr.size(), "%Y/%m/%d %H:%M:%S",
             createTimeTm);
    strftime(modifyTimeStr.data(), modifyTimeStr.size(), "%Y/%m/%d %H:%M:%S",
             modifyTimeTm);

    fileTimes.first = createTimeStr.data();
    fileTimes.second = modifyTimeStr.data();

#endif
    LOG_F(INFO, "getFileTimes returning: createTime: {}, modifyTime: {}",
          fileTimes.first, fileTimes.second);
    return fileTimes;
}

auto checkFileTypeInFolder(const std::string &folderPath,
                           const std::string &fileType,
                           FileOption fileOption) -> std::vector<std::string> {
    LOG_F(INFO,
          "checkFileTypeInFolder called with folderPath: {}, fileType: {}, "
          "fileOption: {}",
          folderPath, fileType, static_cast<int>(fileOption));
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
    LOG_F(INFO, "checkFileTypeInFolder returning {} files", files.size());
    return files;
}

auto isExecutableFile(const std::string &fileName,
                      const std::string &fileExt) -> bool {
    LOG_F(INFO, "isExecutableFile called with fileName: {}, fileExt: {}",
          fileName, fileExt);
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
    LOG_F(INFO, "getFileSize called with filePath: {}", filePath);
    std::size_t size = fs::file_size(filePath);
    LOG_F(INFO, "getFileSize returning: {}", size);
    return size;
}

auto calculateChunkSize(std::size_t fileSize, int numChunks) -> std::size_t {
    LOG_F(INFO, "calculateChunkSize called with fileSize: {}, numChunks: {}",
          fileSize, numChunks);
    std::size_t chunkSize = (fileSize + numChunks - 1) / numChunks;
    LOG_F(INFO, "calculateChunkSize returning: {}", chunkSize);
    return chunkSize;
}

void splitFile(const std::string &filePath, std::size_t chunkSize,
               const std::string &outputPattern) {
    LOG_F(
        INFO,
        "splitFile called with filePath: {}, chunkSize: {}, outputPattern: {}",
        filePath, chunkSize, outputPattern);
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
    LOG_F(INFO, "mergeFiles called with outputFilePath: {}, partFiles size: {}",
          outputFilePath, partFiles.size());
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
    LOG_F(
        INFO,
        "quickSplit called with filePath: {}, numChunks: {}, outputPattern: {}",
        filePath, numChunks, outputPattern);
    std::size_t fileSize = getFileSize(filePath);
    std::size_t chunkSize = calculateChunkSize(fileSize, numChunks);
    splitFile(filePath, chunkSize, outputPattern);
    LOG_F(INFO, "quickSplit completed for filePath: {}", filePath);
}

void quickMerge(const std::string &outputFilePath,
                const std::string &partPattern, int numChunks) {
    LOG_F(INFO,
          "quickMerge called with outputFilePath: {}, partPattern: {}, "
          "numChunks: {}",
          outputFilePath, partPattern, numChunks);
    std::vector<std::string> partFiles;
    for (int i = 0; i < numChunks; ++i) {
        std::ostringstream partFileName;
        partFileName << partPattern << i;
        partFiles.push_back(partFileName.str());
    }
    mergeFiles(outputFilePath, partFiles);
    LOG_F(INFO, "quickMerge completed for outputFilePath: {}", outputFilePath);
}

#ifdef _WIN32
const char PATH_SEPARATORS[] = "/\\";
#else
const char PATH_SEPARATORS[] = "/";
#endif

auto getExecutableNameFromPath(const std::string &path) -> std::string {
    LOG_F(INFO, "getExecutableNameFromPath called with path: {}", path);

    if (path.empty()) {
        LOG_F(ERROR, "The provided path is empty.");
        THROW_INVALID_ARGUMENT("The provided path is empty.");
    }

    size_t lastSlashPos = path.find_last_of(PATH_SEPARATORS);
    LOG_F(INFO, "Last slash position: {}", lastSlashPos);

    if (lastSlashPos == std::string::npos) {
        if (path.find('.') == std::string::npos) {
            LOG_F(ERROR,
                  "The provided path does not contain a valid file name.");
            THROW_INVALID_ARGUMENT(
                "The provided path does not contain a valid file name.");
        }
        LOG_F(INFO, "Returning path as file name: {}", path);
        return path;
    }

    std::string fileName = path.substr(lastSlashPos + 1);
    LOG_F(INFO, "Extracted file name: {}", fileName);

    if (fileName.empty()) {
        LOG_F(ERROR,
              "The provided path ends with a slash and contains no file name.");
        THROW_INVALID_ARGUMENT(
            "The provided path ends with a slash and contains no file name.");
    }

    size_t dotPos = fileName.find_last_of('.');
    LOG_F(INFO, "Last dot position: {}", dotPos);

    if (dotPos == std::string::npos) {
        LOG_F(ERROR, "The file name does not contain an extension.");
        THROW_INVALID_ARGUMENT("The file name does not contain an extension.");
    }

    LOG_F(INFO, "Returning file name: {}", fileName);
    return fileName;
}

auto checkPathType(const fs::path &path) -> PathType {
    if (fs::exists(path)) {
        if (fs::is_regular_file(path)) {
            return PathType::REGULAR_FILE;
        }
        if (fs::is_directory(path)) {
            return PathType::DIRECTORY;
        }
        if (fs::is_symlink(path)) {
            return PathType::SYMLINK;
        }
        return PathType::OTHER;
    }
    return PathType::NOT_EXISTS;
}

auto countLinesInFile(const std::string &filePath) -> std::optional<int> {
    std::ifstream file(filePath);
    int lineCount = 0;
    std::string line;

    if (file.is_open()) {
        while (std::getline(file, line)) {
            lineCount++;
        }
        file.close();
    } else {
        return std::nullopt;
    }
    return lineCount;
}
}  // namespace atom::io
