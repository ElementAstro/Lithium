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
#include <ctime>
#include <filesystem>
#include <iostream>
#include <regex>

#include "atom/log/loguru.hpp"

#ifdef _WIN32
#include <windows.h>
const std::string PATH_SEPARATOR = "\\";
const std::regex folderNameRegex("^[^\\/?*:;{}\\\\]+[^\\\\]*$");
const std::regex fileNameRegex("^[^\\/:*?\"<>|]+$");
#else
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
const std::string PATH_SEPARATOR = "/";
const std::regex folderNameRegex("^[^/]+$");
const std::regex fileNameRegex("^[^/]+$");
#endif

namespace fs = std::filesystem;

#define ATOM_IO_CHECK_ARGUMENT(value)                              \
    if (value.empty()) {                                           \
        LOG_F(ERROR, "{}: Invalid argument: {}", __func__, value); \
        return false;                                              \
    }

#define ATOM_IO_CHECK_ARGUMENT_S(value)                            \
    if (value.empty()) {                                           \
        LOG_F(ERROR, "{}: Invalid argument: {}", __func__, value); \
        return "";                                                 \
    }

namespace Atom::IO {

bool createDirectory(const std::string &path) {
    ATOM_IO_CHECK_ARGUMENT(path);
    try {
        fs::create_directory(path);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to create directory {}: {}", path, e.what());
    }
    return false;
}

bool removeDirectory(const std::string &path) {
    ATOM_IO_CHECK_ARGUMENT(path);
    try {
        fs::remove_all(path);
        DLOG_F(INFO, "Directory removed: {}", path);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to remove directory {}: {}", path, e.what());
    }
    return false;
}

bool renameDirectory(const std::string &old_path, const std::string &new_path) {
    ATOM_IO_CHECK_ARGUMENT(old_path);
    ATOM_IO_CHECK_ARGUMENT(new_path);
    try {
        fs::rename(old_path, new_path);
        DLOG_F(INFO, "Directory renamed from {} to {}", old_path, new_path);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to rename directory from {} to {}: {}", old_path,
              new_path, e.what());
    }
    return false;
}

bool moveDirectory(const std::string &old_path, const std::string &new_path) {
    ATOM_IO_CHECK_ARGUMENT(old_path);
    ATOM_IO_CHECK_ARGUMENT(new_path);
    try {
        fs::rename(old_path, new_path);
        DLOG_F(INFO, "Directory moved from {} to {}", old_path, new_path);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to move directory from {} to {}: {}", old_path,
              new_path, e.what());
    }
    return false;
}

bool copyFile(const std::string &src_path, const std::string &dst_path) {
    ATOM_IO_CHECK_ARGUMENT(src_path);
    ATOM_IO_CHECK_ARGUMENT(dst_path);
    try {
        fs::copy_file(src_path, dst_path);
        DLOG_F(INFO, "File copied from {} to {}", src_path, dst_path);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to copy file from {} to {}: {}", src_path,
              dst_path, e.what());
    }
    return false;
}

bool moveFile(const std::string &src_path, const std::string &dst_path) {
    ATOM_IO_CHECK_ARGUMENT(src_path);
    ATOM_IO_CHECK_ARGUMENT(dst_path);
    try {
        fs::rename(src_path, dst_path);
        DLOG_F(INFO, "File moved from {} to {}", src_path, dst_path);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to move file from {} to {}: {}", src_path,
              dst_path, e.what());
    }
    return false;
}

bool renameFile(const std::string &old_path, const std::string &new_path) {
    ATOM_IO_CHECK_ARGUMENT(old_path);
    ATOM_IO_CHECK_ARGUMENT(new_path);
    try {
        fs::rename(old_path, new_path);
        DLOG_F(INFO, "File renamed from {} to {}", old_path, new_path);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to rename file from {} to {}: {}", old_path,
              new_path, e.what());
    }
    return false;
}

bool removeFile(const std::string &path) {
    ATOM_IO_CHECK_ARGUMENT(path);
    try {
        fs::remove(path);
        DLOG_F(INFO, "File removed: {}", path);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to remove file {}: {}", path, e.what());
    }
    return false;
}

bool createSymlink(const std::string &target_path,
                   const std::string &symlink_path) {
    ATOM_IO_CHECK_ARGUMENT(target_path);
    ATOM_IO_CHECK_ARGUMENT(symlink_path);
    try {
        fs::create_symlink(target_path, symlink_path);
        DLOG_F(INFO, "Symlink created from {} to {}", target_path,
               symlink_path);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to create symlink from {} to {}: {}", target_path,
              symlink_path, e.what());
    }
    return false;
}

bool removeSymlink(const std::string &path) {
    ATOM_IO_CHECK_ARGUMENT(path);
    try {
        fs::remove(path);
        DLOG_F(INFO, "Symlink removed: {}", path);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to remove symlink {}: {}", path, e.what());
    }
    return false;
}

std::uintmax_t fileSize(const std::string &path) {
    try {
        return fs::file_size(path);
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to get file size of {}: {}", path, e.what());
        return 0;
    }
}

std::string convertToLinuxPath(const std::string &windows_path) {
    ATOM_IO_CHECK_ARGUMENT_S(windows_path);
    std::string linux_path = windows_path;
    for (char &c : linux_path) {
        if (c == '\\') {
            c = '/';
        }
    }
    if (linux_path.length() >= 2 && linux_path[1] == ':') {
        linux_path[0] = tolower(linux_path[0]);
    }
    return linux_path;
}

std::string convertToWindowsPath(const std::string &linux_path) {
    ATOM_IO_CHECK_ARGUMENT_S(linux_path);
    std::string windows_path = linux_path;
    for (char &c : windows_path) {
        if (c == '/') {
            c = '\\';
        }
    }
    if (windows_path.length() >= 2 && islower(windows_path[0]) &&
        windows_path[1] == ':') {
        windows_path[0] = toupper(windows_path[0]);
    }
    return windows_path;
}

std::string getAbsoluteDirectory() {
    fs::path program_path;
#ifdef _WIN32
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    program_path = buffer;
#else
    char buffer[PATH_MAX];
    ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer));
    if (length != -1) {
        program_path = std::string(buffer, length);
    }
#endif
    return program_path.parent_path().string();
}

std::string normalizePath(const std::string &path) {
    std::string normalized_path = path;
    std::replace(normalized_path.begin(), normalized_path.end(), '/',
                 PATH_SEPARATOR.front());
    std::replace(normalized_path.begin(), normalized_path.end(), '\\',
                 PATH_SEPARATOR.front());
    return normalized_path;
}

void traverseDirectories(const fs::path &directory,
                         std::vector<std::string> &folders) {
    DLOG_F(INFO, "Traversing directory: {}", directory.string());
    for (const auto &entry : fs::directory_iterator(directory)) {
        if (entry.is_directory()) {
            std::string folder_path = normalizePath(entry.path().string());
            folders.push_back(folder_path);
            traverseDirectories(entry.path(), folders);
        }
    }
}

bool isFolderNameValid(const std::string &folderName) {
    ATOM_IO_CHECK_ARGUMENT(folderName);
    return std::regex_match(folderName, folderNameRegex);
}

bool isFileNameValid(const std::string &fileName) {
    ATOM_IO_CHECK_ARGUMENT(fileName);
    return std::regex_match(fileName, fileNameRegex);
}

bool isFolderExists(const std::string &folderName) {
    if (!isFolderNameValid(folderName)) {
        return false;
    }
    return fs::exists(folderName) && fs::is_directory(folderName);
}

bool isFileExists(const std::string &fileName) {
    if (!isFileNameValid(fileName)) {
        return false;
    }
    return fs::exists(fileName) && fs::is_regular_file(fileName);
}

bool isFolderEmpty(const std::string &folderName) {
    if (!isFolderExists(folderName)) {
        return false;
    }
    fs::path directory_path = folderName;
    for (const auto &entry : fs::directory_iterator(directory_path)) {
        if (fs::is_regular_file(entry)) {
            return true;
        }
    }
    return false;
}

bool isAbsolutePath(const std::string &path) {
    return std::filesystem::path(path).is_absolute();
}

std::string normPath(const std::string &path) {
    std::vector<std::string> components;
    std::istringstream iss(path);
    std::string component;

    // 分割路径为组件
    while (std::getline(iss, component, '/')) {
        if (component == "" || component == ".") {
            continue;  // 忽略空和当前目录符号
        } else if (component == "..") {
            if (!components.empty() && components.back() != "..") {
                components.pop_back();  // 弹出上一级目录符号
            } else {
                components.push_back("..");  // 保留多余的上一级目录符号
            }
        } else {
            components.push_back(component);  // 添加有效组件
        }
    }

    // 重新组合路径
    std::string result;
    for (const std::string &comp : components) {
        result += "/" + comp;
    }

    return result.empty() ? "/" : result;
}

bool changeWorkingDirectory(const std::string &directoryPath) {
    if (!isFolderNameValid(directoryPath) || !isFolderExists(directoryPath)) {
        LOG_F(ERROR, "Directory does not exist: {}", directoryPath);
        return false;
    }
    try {
        fs::current_path(directoryPath);
        return true;
    } catch (const std::filesystem::filesystem_error &e) {
        LOG_F(ERROR, "Failed to change working directory: {}", e.what());
        return false;
    }
}

std::pair<std::string, std::string> getFileTimes(const std::string &filePath) {
    std::pair<std::string, std::string> fileTimes;

#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA fileInfo;
    if (!GetFileAttributesEx(filePath.c_str(), GetFileExInfoStandard,
                             &fileInfo)) {
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
    sprintf_s(createTimeStr, "%04d/%02d/%02d %02d:%02d:%02d",
              createSysTime.wYear, createSysTime.wMonth, createSysTime.wDay,
              createSysTime.wHour, createSysTime.wMinute,
              createSysTime.wSecond);
    sprintf_s(modifyTimeStr, "%04d/%02d/%02d %02d:%02d:%02d",
              modifySysTime.wYear, modifySysTime.wMonth, modifySysTime.wDay,
              modifySysTime.wHour, modifySysTime.wMinute,
              modifySysTime.wSecond);

    fileTimes.first = createTimeStr;
    fileTimes.second = modifyTimeStr;

#else
    struct stat fileInfo;
    if (stat(filePath.c_str(), &fileInfo) != 0) {
        LOG_F(ERROR, "Error getting file information.");
        return fileTimes;
    }

    std::time_t createTime = fileInfo.st_ctime;
    std::time_t modifyTime = fileInfo.st_mtime;

    struct tm *createTimeTm = localtime(&createTime);
    struct tm *modifyTimeTm = localtime(&modifyTime);

    char createTimeStr[20], modifyTimeStr[20];
    strftime(createTimeStr, sizeof(createTimeStr), "%Y/%m/%d %H:%M:%S",
             createTimeTm);
    strftime(modifyTimeStr, sizeof(modifyTimeStr), "%Y/%m/%d %H:%M:%S",
             modifyTimeTm);

    fileTimes.first = createTimeStr;
    fileTimes.second = modifyTimeStr;

#endif

    return fileTimes;
}

std::vector<std::string> checkFileTypeInFolder(const std::string &folderPath,
                                               const std::string &fileType,
                                               FileOption fileOption) {
    std::vector<std::string> files;

    try {
        for (const auto &entry :
             std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_regular_file() &&
                entry.path().extension() == fileType) {
                if (fileOption == FileOption::Path) {
                    files.push_back(entry.path().string());
                } else if (fileOption == FileOption::Name) {
                    files.push_back(entry.path().filename().string());
                }
            }
        }
    } catch (const std::filesystem::filesystem_error &ex) {
        LOG_F(ERROR, "Failed to check files in folder: {}", ex.what());
    }

    return files;
}
}  // namespace Atom::IO
