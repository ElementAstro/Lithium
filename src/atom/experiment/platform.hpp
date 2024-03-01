/*
 * platform.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: A platform information collection.

**************************************************/

#ifndef ATOM_EXPERIMENT_PLATFORM_HPP
#define ATOM_EXPERIMENT_PLATFORM_HPP

// 获取系统平台
#if defined(_WIN32)
    const std::string platform = "Windows";
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

#if defined(_WIN32)
std::string getWindowsVersion();
#endif
// 获取操作系统版本
const std::string os_version = 
#if defined(_WIN32)
    getWindowsVersion();
#elif defined(__APPLE__)
    "macOS"; // 可以根据需要获取更详细的版本信息
#elif defined(__ANDROID__)
    "Android"; // 可以根据需要获取更详细的版本信息
#elif defined(__linux__)
    "Linux"; // 可以根据需要获取更详细的版本信息
#else
    "Unknown OS version";
#endif

// 获取编译器信息
const std::string compiler = 
#if defined(__clang__)
    "Clang " + std::to_string(__clang_major__) + "." + std::to_string(__clang_minor__) + "." + std::to_string(__clang_patchlevel__);
#elif defined(__GNUC__)
    "GCC " + std::to_string(__GNUC__) + "." + std::to_string(__GNUC_MINOR__) + "." + std::to_string(__GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
    "MSVC " + std::to_string(_MSC_FULL_VER);
#else
    "Unknown compiler";
#endif

bool hasGUI();

#endif