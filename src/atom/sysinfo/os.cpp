/*
 * os.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-21

Description: System Information Module - OS Information

**************************************************/

#include "atom/sysinfo/os.hpp"

#include <fstream>
#include <optional>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <unistd.h>
#include <fstream>
#elif __APPLE__
#include <sys/utsname.h>
#endif

#include "atom/log/loguru.hpp"

namespace atom::system {
std::string OperatingSystemInfo::toJson() const {
    std::stringstream ss;
    ss << "{\n";
    ss << R"(  "osName": ")" << osName << "\",\n";
    ss << R"(  "osVersion": ")" << osVersion << "\",\n";
    ss << R"(  "kernelVersion": ")" << kernelVersion << "\"\n";
    ss << R"(  "architecture": ")" << architecture << "\"\n";
    ss << "}\n";
    return ss.str();
}

std::optional<std::string> getComputerName() {
    char buffer[256];

#if defined(_WIN32)
    DWORD size = sizeof(buffer);
    if (GetComputerNameA(buffer, &size)) {
        return std::string(buffer);
    }
#elif defined(__APPLE__)
    CFStringRef name = SCDynamicStoreCopyComputerName(NULL, NULL);
    if (name != NULL) {
        CFStringGetCString(name, buffer, sizeof(buffer), kCFStringEncodingUTF8);
        CFRelease(name);
        return std::string(buffer);
    }
#elif defined(__linux__) || defined(__linux)
    if (gethostname(buffer, sizeof(buffer)) == 0) {
        return std::string(buffer);
    }
#elif defined(__ANDROID__)
    return std::nullopt;
#endif

    return std::nullopt;
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
                osInfo.osName = line.substr(line.find('=') + 1);
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
    const std::string ARCHITECTURE = "x86";
#elif defined(__x86_64__)
    const std::string ARCHITECTURE = "x86_64";
#elif defined(__arm__)
    const std::string ARCHITECTURE = "ARM";
#elif defined(__aarch64__)
    const std::string ARCHITECTURE = "ARM64";
#else
    const std::string ARCHITECTURE = "Unknown architecture";
#endif
    osInfo.architecture = ARCHITECTURE;

    const std::string COMPILER =
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
    osInfo.compiler = COMPILER;

    osInfo.computerName = getComputerName().value_or("Unknown computer name");

    return osInfo;
}

auto isWsl() -> bool {
    std::ifstream procVersion("/proc/version");
    std::string line;
    if (procVersion.is_open()) {
        std::getline(procVersion, line);
        procVersion.close();
        // Check if the line contains "Microsoft" which is a typical indicator of WSL
        return line.find("microsoft") != std::string::npos || line.find("WSL") != std::string::npos;
    }
    return false;
}

}  // namespace atom::system
