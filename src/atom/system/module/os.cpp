/*
 * os.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - OS Information

**************************************************/

#include "os.hpp"

#include <sstream>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <fstream>
#elif __APPLE__
#include <sys/utsname.h>
#endif

#include "atom/log/loguru.hpp"

namespace atom::system {
std::string OperatingSystemInfo::toJson() const {
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"osName\": \"" << osName << "\",\n";
    ss << "  \"osVersion\": \"" << osVersion << "\",\n";
    ss << "  \"kernelVersion\": \"" << kernelVersion << "\"\n";
    ss << "  \"architecture\": \"" << architecture << "\"\n";
    ss << "}\n";
    return ss.str();
}

OperatingSystemInfo getOperatingSystemInfo() {
    OperatingSystemInfo osInfo;

#ifdef _WIN32
    OSVERSIONINFOEX osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if (GetVersionEx((LPOSVERSIONINFO)&osvi) != 0) {
        osInfo.osName = "Windows";
        osInfo.osVersion = std::to_string(osvi.dwMajorVersion) + "." +
                           std::to_string(osvi.dwMinorVersion) + " (Build " +
                           std::to_string(osvi.dwBuildNumber) + ")";
    } else {
        LOG_F(ERROR, "Failed to get OS version");
    }
#elif __linux__
    std::ifstream osReleaseFile("/etc/os-release");
    if (osReleaseFile.is_open()) {
        std::string line;
        while (std::getline(osReleaseFile, line)) {
            if (line.find("PRETTY_NAME") != std::string::npos) {
                osInfo.osName = line.substr(line.find("=") + 1);
                break;
            }
        }
        osReleaseFile.close();
    }
    if (osInfo.osName.empty()) {
        LOG_F(ERROR, "Failed to get OS name");
    }

    std::ifstream kernelVersionFile("/proc/version");
    if (kernelVersionFile.is_open()) {
        std::string line;
        std::getline(kernelVersionFile, line);
        osInfo.kernelVersion = line.substr(0, line.find(" "));
        kernelVersionFile.close();
    } else {
        LOG_F(ERROR, "Failed to open /proc/version");
    }
#elif __APPLE__
    struct utsname info;
    if (uname(&info) == 0) {
        osInfo.osName = info.sysname;
        osInfo.osVersion = info.release;
        osInfo.kernelVersion = info.version;
    }
#endif

// 获取系统架构
#if defined(__i386__) || defined(__i386)
    const std::string architecture = "x86";
#elif defined(__x86_64__)
    const std::string architecture = "x86_64";
#elif defined(__arm__)
    const std::string architecture = "ARM";
#elif defined(__aarch64__)
    const std::string architecture = "ARM64";
#else
    const std::string architecture = "Unknown architecture";
#endif
    osInfo.architecture = architecture;

    const std::string compiler =
#if defined(__clang__)
        "Clang " + std::to_string(__clang_major__) + "." +
        std::to_string(__clang_minor__) + "." +
        std::to_string(__clang_patchlevel__);
#elif defined(__GNUC__)
        "GCC " + std::to_string(__GNUC__) + "." +
        std::to_string(__GNUC_MINOR__) + "." +
        std::to_string(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
        "MSVC " + std::to_string(_MSC_FULL_VER);
#else
        "Unknown compiler";
#endif
    osInfo.compiler = compiler;

    return osInfo;
}
}  // namespace atom::system