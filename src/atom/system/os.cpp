/*
 * os.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-4-4

Description: Some useful system functions from Python.

**************************************************/

#include "os.hpp"

#include <sys/stat.h>
#include <fstream>
#include <string>
#ifdef _WIN32
#include <Windows.h>
#include <io.h>
#include <lmcons.h>
#else
#include <unistd.h>
#endif

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

#include "atom/io/io.hpp"

using json = nlohmann::json;

namespace atom::system {
void walk(const fs::path &root) {
    for (const auto &entry : fs::directory_iterator(root)) {
        if (fs::is_directory(entry)) {
            DLOG_F(INFO, "Directory: {}", entry.path().generic_string());
            walk(entry);
        } else {
            DLOG_F(INFO, "File: {}", entry.path().generic_string());
        }
    }
}

json walk(const fs::path &path, bool recursive) {
    DLOG_F(INFO, "Walking: {}", path.generic_string());
    if (!fs::exists(path)) {
        LOG_F(ERROR, "Path does not exist: {}", path.generic_string());
        return json();
    }

    json folder = {{"path", path.generic_string()},
                   {"directories", json::array()},
                   {"files", json::array()}};

    for (const auto &entry : fs::directory_iterator(path)) {
        if (fs::is_directory(entry)) {
            DLOG_F(INFO, "Directory: {}", entry.path().generic_string());
            folder["directories"].push_back(walk(entry.path(), true));
        } else {
            DLOG_F(INFO, "File: {}", entry.path().generic_string());
            folder["files"].push_back(entry.path().generic_string());
        }
    }

    return folder;
}

std::string jwalk(const std::string &root) {
    DLOG_F(INFO, "Walking: {}", root);
    if (!fs::exists(root)) {
        LOG_F(ERROR, "Directory not exists: {}", root);
        return "";
    }
    json folder = {{"path", root},
                   {"directories", json::array()},
                   {"files", json::array()}};

    for (const auto &entry : fs::directory_iterator(root)) {
        if (fs::is_directory(entry)) {
            DLOG_F(INFO, "Directory: {}", entry.path().generic_string());
            folder["directories"].push_back(walk(entry, true));
        } else {
            DLOG_F(INFO, "File: {}", entry.path().generic_string());
            folder["files"].push_back(entry.path().generic_string());
        }
    }

    return folder.dump();
}

void fwalk(const fs::path &root,
           const std::function<void(const fs::path &)> &callback) {
    for (const auto &entry : fs::directory_iterator(root)) {
        if (fs::is_directory(entry)) {
            fwalk(entry, callback);
        } else {
            callback(entry.path());
        }
    }
}

bool truncate(const std::string &path, std::streamsize size) {
    std::ofstream file(path,
                       std::ios::out | std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    file.seekp(size);
    file.put('\0');
    return true;
}

std::vector<fs::path> scandir(const fs::path &path) {
    std::vector<fs::path> entries;
    for (const auto &entry : fs::directory_iterator(path)) {
        entries.push_back(entry.path());
    }
    return entries;
}

mode_t umask(mode_t mask) {
#ifdef _WIN32
    int oldMask;
    _umask_s(mask, &oldMask);
    return static_cast<mode_t>(oldMask);
#else
    // 类Unix系统
    return ::umask(mask);
#endif
}

int getpriority() {
#ifdef _WIN32
    // Windows平台
    return GetPriorityClass(GetCurrentProcess());
#else
    // 类Unix系统
    return ::getpriority(PRIO_PROCESS, 0);
#endif
}

std::string getlogin() {
#ifdef _WIN32
    // Windows平台
    char buffer[UNLEN + 1];
    DWORD bufferSize = UNLEN + 1;
    if (GetUserNameA(buffer, &bufferSize)) {
        return buffer;
    }
#else
    // 类Unix系统
    char *username = ::getlogin();
    if (username != nullptr) {
        return username;
    }
#endif
    return "";
}

std::unordered_map<std::string, std::string> Environ() {
    std::unordered_map<std::string, std::string> env;

#ifdef _WIN32
    // Windows平台
    LPWCH variables = GetEnvironmentStringsW();
    if (variables != nullptr) {
        LPWSTR currentVariable = variables;
        while (*currentVariable != L'\0') {
            std::wstring wideString(currentVariable);
            std::string variable(wideString.begin(), wideString.end());
            size_t delimiterPos = variable.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = variable.substr(0, delimiterPos);
                std::string value = variable.substr(delimiterPos + 1);
                env[key] = value;
            }
            currentVariable += wideString.size() + 1;
        }

        FreeEnvironmentStringsW(variables);
    }
#else
    // 类Unix系统
    extern char **environ;
    char **currentVariable = environ;
    while (*currentVariable != nullptr) {
        std::string variable(*currentVariable);
        size_t delimiterPos = variable.find('=');
        if (delimiterPos != std::string::npos) {
            std::string key = variable.substr(0, delimiterPos);
            std::string value = variable.substr(delimiterPos + 1);
            env[key] = value;
        }
        ++currentVariable;
    }
#endif

    return env;
}

std::string ctermid() {
#ifdef _WIN32
    // Windows平台
    const int BUFFER_SIZE = 256;
    char buffer[BUFFER_SIZE];
    DWORD length = GetConsoleTitleA(buffer, BUFFER_SIZE);
    if (length > 0) {
        return std::string(buffer, length);
    }
#else
    // 类Unix系统
    char buffer[L_ctermid];
    if (::ctermid(buffer) != nullptr) {
        return buffer;
    }
#endif
    return "";
}

Utsname uname() {
    Utsname info;

#ifdef _WIN32
    // Windows平台
    OSVERSIONINFOA osVersionInfo;
    ZeroMemory(&osVersionInfo, sizeof(OSVERSIONINFOA));
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    if (GetVersionExA(&osVersionInfo)) {
        info.sysname = "Windows";
        info.nodename = "";
        info.release = std::to_string(osVersionInfo.dwMajorVersion) + "." +
                       std::to_string(osVersionInfo.dwMinorVersion);
        info.version = std::to_string(osVersionInfo.dwBuildNumber);
        info.machine = osVersionInfo.szCSDVersion;
    }
#else
    // 类Unix系统
    struct utsname uts;
    if (::uname(&uts) == 0) {
        info.sysname = uts.sysname;
        info.nodename = uts.nodename;
        info.release = uts.release;
        info.version = uts.version;
        info.machine = uts.machine;
    }
#endif

    return info;
}
}  // namespace atom::system
