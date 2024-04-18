/*
 * constants.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Constants for Lithium

**************************************************/

#ifndef LITHIUM_UTILS_CONSTANTS_HPP
#define LITHIUM_UTILS_CONSTANTS_HPP

#include <string>
#include <vector>

class constants {
public:
#ifdef _WIN32
#if defined(__MINGW32__) || defined(__MINGW64__)
    static constexpr const char* PATH_SEPARATOR = "/";
    static constexpr const char* LIB_EXTENSION = ".dll";
#else
    static constexpr const char* PATH_SEPARATOR = "\\";
    static constexpr const char* LIB_EXTENSION = ".dll";
#endif
#elif defined(__APPLE__)
    static constexpr const char* PATH_SEPARATOR = "/";
    static constexpr const char* LIB_EXTENSION = ".dylib";
#else
    static constexpr const char* PATH_SEPARATOR = "/";
    static constexpr const char* LIB_EXTENSION = ".so";
#endif

    // Package info
    static constexpr const char* PACKAGE_NAME = "package.json";
    static constexpr const char* PACKAGE_NAME_SHORT = "lithium";
    static constexpr const char* PACKAGE_VERSION = "0.1.0";

    // Module info
#ifdef _WIN32
#if defined(__MINGW32__) || defined(__MINGW64__)
    static constexpr const char* MODULE_FOLDER = "./modules";
#else
    static constexpr const char* MODULE_FOLDER = ".\\modules";
#endif
#else
    static constexpr const char* MODULE_FOLDER = "./modules";
#endif

#ifdef _WIN32
    static constexpr const char* COMPILER = "cl.exe";
#elif __APPLE__
    static constexpr const char* COMPILER = "clang++";
#else
    static constexpr const char* COMPILER = "g++";
#endif

#ifdef _WIN32
    static const std::vector<std::string> COMMON_COMPILERS = {
        "cl.exe", "g++.exe", "clang++.exe"};
    static const std::vector<std::string> COMPILER_PATHS = {
        "C:\\Program Files (x86)\\Microsoft Visual "
        "Studio\\2019\\Community\\VC\\Tools\\MSVC\\14.29."
        "30133\\bin\\Hostx64\\x64",
        "C:\\Program Files\\Microsoft Visual "
        "Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.35."
        "32215\\bin\\Hostx64\\x64",
        "C:\\msys64\\mingw64\\bin", "C:\\MinGW\\bin",
        "C:\\Program Files\\LLVM\\bin"};
#elif __APPLE__
    static const std::vector<std::string> COMMON_COMPILERS = {"clang++", "g++"};
    static const std::vector<std::string> COMPILER_PATHS = {
        "/usr/bin", "/usr/local/bin", "/opt/local/bin"};
#elif __linux__
    static const std::vector<std::string> COMMON_COMPILERS = {"g++", "clang++"};
    static const std::vector<std::string> COMPILER_PATHS = {"/usr/bin",
                                                            "/usr/local/bin"};
#endif

    // Env info
    static constexpr const char* ENV_VAR_MODULE_PATH = "LITHIUM_MODULE_PATH";

    // Inside Module Identifiers

    static constexpr const char* LITHIUM_MODULE_LOADER = "lithium.addon.loader";
    static constexpr const char* LITHIUM_ADDON_MANAGER = "lithium.addon.addon";
    static constexpr const char* LITHIUM_UTILS_ENV = "lithium.utils.env";
};

#endif  // LITHIUM_UTILS_CONSTANTS_HPP
