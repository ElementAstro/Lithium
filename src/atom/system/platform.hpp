/*
 * platform.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: A platform information collection.

**************************************************/

#ifndef ATOM_SYSTEM_PLATFORM_HPP
#define ATOM_SYSTEM_PLATFORM_HPP

#include <string>

namespace atom::system {
#if defined(_WIN32)
#if defined(__MINGW32__) || defined(__MINGW64__)
const std::string platform = "Windows MinGW";
#else
const std::string platform = "Windows MSVC";
#endif
#elif defined(__APPLE__)
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
const std::string platform = "iOS Simulator";
#elif TARGET_OS_IPHONE
const std::string platform = "iOS";
#elif TARGET_OS_MAC
const std::string platform = "macOS";
#else
const std::string platform = "Unknown Apple platform";
#endif
#elif defined(__ANDROID__)
const std::string platform = "Android";
#elif defined(__linux__)
const std::string platform = "Linux";
#else
const std::string platform = "Unknown platform";
#endif

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

#ifdef _WIN32
std::string getWindowsVersion();
#endif
const std::string os_version =
#ifdef _WIN32
    getWindowsVersion();
#elif defined(__APPLE__)
    "macOS";
#elif defined(__ANDROID__)
    "Android";
#elif defined(__linux__)
#if defined(__ANDROID__)
#include <android/api-level.h>
#if __ANDROID_API__ >= 21
    "Android " + std::to_string(__ANDROID_API__);
#else
    "Android";
#endif
#else
    "Linux";
#endif
#else
    "Unknown OS version";
#endif

const std::string compiler =
#if defined(__clang__)
    "Clang " + std::to_string(__clang_major__) + "." +
    std::to_string(__clang_minor__) + "." +
    std::to_string(__clang_patchlevel__);
#elif defined(__GNUC__)
    "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) +
    "." + std::to_string(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    "MSVC " + std::to_string(_MSC_FULL_VER);
#else
    "Unknown compiler";
#endif

bool hasGUI();
}  // namespace atom::system

#endif
