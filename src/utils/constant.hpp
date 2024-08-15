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
#else
    static constexpr const char* PATH_SEPARATOR = "\\";
#endif
    static constexpr const char* LIB_EXTENSION = ".dll";
    static constexpr const char* EXECUTABLE_EXTENSION = ".exe";
#elif defined(__APPLE__)
    static constexpr const char* PATH_SEPARATOR = "/";
    static constexpr const char* LIB_EXTENSION = ".dylib";
    static constexpr const char* EXECUTABLE_EXTENSION = "";
#else
    static constexpr const char* PATH_SEPARATOR = "/";
    static constexpr const char* LIB_EXTENSION = ".so";
    static constexpr const char* EXECUTABLE_EXTENSION = "";
#endif

    // Package info
    static constexpr const char* PACKAGE_NAME = "package.json";
    static constexpr const char* PACKAGE_NAME_SHORT = "lithium";
    static constexpr const char* PACKAGE_VERSION = "0.1.0";

    // Module info
#ifdef _WIN32
#if defined(__MINGW32__) || defined(__MINGW64__)
    static constexpr const char* MODULE_FOLDER = "./modules";
    static constexpr const char* COMPILER = "g++";
    static constexpr const char* TASK_FOLDER = "./tasks";
#else
    static constexpr const char* MODULE_FOLDER = ".\\modules";
    static constexpr const char* COMPILER = "cl.exe";
    static constexpr const char* TASK_FOLDER = ".\\tasks";
#endif
#elif defined(__APPLE__)
    static constexpr const char* MODULE_FOLDER = "./modules";
    static constexpr const char* COMPILER = "clang++";
    static constexpr const char* TASK_FOLDER = "./tasks";
#else
    static constexpr const char* MODULE_FOLDER = "./modules";
    static constexpr const char* COMPILER = "g++";
    static constexpr const char* TASK_FOLDER = "./tasks";
#endif

    static std::vector<std::string> COMMON_COMPILERS;
    static std::vector<std::string> COMPILER_PATHS;

    // Env info
    static constexpr const char* ENV_VAR_MODULE_PATH = "LITHIUM_MODULE_PATH";

    // Inside Module Identifiers
    static constexpr const char* LITHIUM_COMPONENT_MANAGER =
        "lithium.addon.manager";
    static constexpr const char* LITHIUM_MODULE_LOADER = "lithium.addon.loader";
    static constexpr const char* LITHIUM_ADDON_MANAGER = "lithium.addon.addon";
    static constexpr const char* LITHIUM_UTILS_ENV = "lithium.utils.env";

    static constexpr const char* LITHIUM_PROCESS_MANAGER =
        "lithium.system.process";

    static constexpr const char* LITHIUM_DEVICE_LOADER =
        "lithium.device.loader";
    static constexpr const char* LITHIUM_DEVICE_MANAGER =
        "lithium.device.manager";

    static std::vector<std::string> LITHIUM_RESOURCES;
    static std::vector<std::string_view> LITHIUM_RESOURCES_SHA256;

    // Task
    static constexpr const char* LITIHUM_TASK_MANAGER = "lithium.task.manager";
    static constexpr const char* LITHIUM_TASK_CONTAINER =
        "lithium.task.container";
    static constexpr const char* LITHIUM_TASK_POOL = "lithium.task.pool";
    static constexpr const char* LITHIUM_TASK_LIST = "lithium.task.list";
    static constexpr const char* LITHIUM_TASK_GENERATOR =
        "lithium.task.generator";

    static constexpr const char* LITHIUM_COMMAND = "lithium.command";
};

#endif  // LITHIUM_UTILS_CONSTANTS_HPP
