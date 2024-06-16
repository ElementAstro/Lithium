/*
 * stat.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Python like stat for Windows & Linux

**************************************************/

#include "stat.hpp"

#include <sys/stat.h>
#include <sys/types.h>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "atom/utils/string.hpp"

namespace atom::system {
Stat::Stat(const fs::path& path) : path_(path) { update(); }

void Stat::update() {
    [[maybe_unused]] fs::file_status status = fs::status(path_, ec_);
    if (ec_) {
        throw std::filesystem::filesystem_error("Failed to get file status",
                                                path_, ec_);
    }
}

fs::file_type Stat::type() const { return fs::status(path_).type(); }

std::uintmax_t Stat::size() const { return fs::file_size(path_); }

std::time_t Stat::atime() const {
    using namespace std::chrono;
    auto fileTime = fs::last_write_time(path_);
    auto duration = fileTime.time_since_epoch();
    auto sysTime = system_clock::time_point(duration);
    return system_clock::to_time_t(sysTime);
}

std::time_t Stat::mtime() const {
    return std::chrono::system_clock::to_time_t(
        std::chrono::clock_cast<std::chrono::system_clock>(
            fs::last_write_time(path_)));
}

std::time_t Stat::ctime() const {
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA attr;
    if (!GetFileAttributesEx(
            atom::utils::stringToWString(path_.string()).c_str(),
            GetFileExInfoStandard, &attr)) {
        throw std::system_error(
            std::error_code(GetLastError(), std::system_category()),
            "Failed to get file attributes");
    }
    auto fileTime = FILETIME{attr.ftCreationTime};
    auto fileTimeNS = std::chrono::nanoseconds{
        (std::uint64_t(fileTime.dwHighDateTime) << 32) |
        fileTime.dwLowDateTime};
    auto sysTime =
        std::chrono::time_point<std::chrono::system_clock>(fileTimeNS);
    auto sysTimeCast =
        std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            sysTime);
    return std::chrono::system_clock::to_time_t(sysTimeCast);

#else
    struct stat attr;
    if (stat(path_.c_str(), &attr) != 0) {
        throw std::system_error(std::error_code(errno, std::system_category()),
                                "Failed to get file attributes");
    }
    return attr.st_ctime;
#endif
}

int Stat::mode() const {
#ifdef _WIN32
    return 0;
#else
    struct stat attr;
    if (stat(path_.c_str(), &attr) != 0) {
        throw std::system_error(std::error_code(errno, std::system_category()),
                                "Failed to get file attributes");
    }
    return attr.st_mode;
#endif
}

int Stat::uid() const {
#ifdef _WIN32
    return 0;
#else
    struct stat attr;
    if (stat(path_.c_str(), &attr) != 0) {
        throw std::system_error(std::error_code(errno, std::system_category()),
                                "Failed to get file attributes");
    }
    return attr.st_uid;
#endif
}

int Stat::gid() const {
#ifdef _WIN32
    return 0;
#else
    struct stat attr;
    if (stat(path_.c_str(), &attr) != 0) {
        throw std::system_error(std::error_code(errno, std::system_category()),
                                "Failed to get file attributes");
    }
    return attr.st_gid;
#endif
}

fs::path Stat::path() const { return path_; }
}  // namespace atom::system
