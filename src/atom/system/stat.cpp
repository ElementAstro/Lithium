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
#include "error/exception.hpp"
#include "macro.hpp"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "atom/utils/string.hpp"

namespace atom::system {
Stat::Stat(const fs::path& path) : path_(path) { update(); }

void Stat::update() {
     fs::file_status status = fs::status(path_, ec_);
     ATOM_UNUSED_RESULT(status)
    if (ec_) {
        THROW_FAIL_TO_OPEN_FILE("Failed to get file status",
                                                path_, ec_);
    }
}

auto Stat::type() const -> fs::file_type { return fs::status(path_).type(); }

auto Stat::size() const -> std::uintmax_t { return fs::file_size(path_); }

auto Stat::atime() const -> std::time_t {
    using namespace std::chrono;
    auto fileTime = fs::last_write_time(path_);
    auto duration = fileTime.time_since_epoch();
    auto sysTime = system_clock::time_point(duration);
    return system_clock::to_time_t(sysTime);
}

auto Stat::mtime() const -> std::time_t {
    return std::chrono::system_clock::to_time_t(
        std::chrono::clock_cast<std::chrono::system_clock>(
            fs::last_write_time(path_)));
}

auto Stat::ctime() const -> std::time_t {
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA attr;
    if (GetFileAttributesExW(
            atom::utils::stringToWString(path_.string()).c_str(),
            GetFileExInfoStandard, &attr) == 0) {
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

auto Stat::mode() const -> int {
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

auto Stat::uid() const -> int {
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

auto Stat::gid() const -> int {
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

auto Stat::path() const -> fs::path { return path_; }
}  // namespace atom::system
