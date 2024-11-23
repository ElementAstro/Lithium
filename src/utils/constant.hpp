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

#include "config.h"

#include <string>
#include <vector>

#include "atom/algorithm/hash.hpp"

#define DEFINE_CONSTANT(name, value) static constexpr const char* name = value;

#define DEFINE_LITHIUM_CONSTANT(name)                     \
    static constexpr const char* name = "lithium." #name; \
    static constexpr std::size_t name##_hash = atom::algorithm::hash(name);

class Constants {
public:
#ifdef _WIN32
#if defined(__MINGW32__) || defined(__MINGW64__)
    DEFINE_CONSTANT(PATH_SEPARATOR, "/")
#else
    DEFINE_CONSTANT(PATH_SEPARATOR, "\\")
#endif
    DEFINE_CONSTANT(LIB_EXTENSION, ".dll")
    DEFINE_CONSTANT(EXECUTABLE_EXTENSION, ".exe")
#elif defined(__APPLE__)
    DEFINE_CONSTANT(PATH_SEPARATOR, "/")
    DEFINE_CONSTANT(LIB_EXTENSION, ".dylib")
    DEFINE_CONSTANT(EXECUTABLE_EXTENSION, "")
#else
    DEFINE_CONSTANT(PATH_SEPARATOR, "/")
    DEFINE_CONSTANT(LIB_EXTENSION, ".so")
    DEFINE_CONSTANT(EXECUTABLE_EXTENSION, "")
#endif

    // Package info
    DEFINE_CONSTANT(PACKAGE_NAME, "package.json")
    DEFINE_CONSTANT(PACKAGE_NAME_SHORT, "lithium")
    DEFINE_CONSTANT(PACKAGE_AUTHOR, "Max Qian")
    DEFINE_CONSTANT(PACKAGE_AUTHOR_EMAIL, "astro_air@126.com")
    DEFINE_CONSTANT(PACKAGE_LICENSE, "AGPL-3")
    DEFINE_CONSTANT(PACKAGE_VERSION, "0.1.0")

    DEFINE_CONSTANT(COMPONENT_PATH, "./modules")
    DEFINE_CONSTANT(COMPONENT_PATH_ENV, "LITHIUM_COMPONENT_PATH")
    DEFINE_CONSTANT(COMPONENT_STATUS_FILE, "./modules/status.json")
    DEFINE_CONSTANT(COMPONENT_STATUS_FILE_ENV, "LITHIUM_COMPONENT_STATUS_FILE")

    static std::vector<std::string> COMMON_COMPILERS;
    static std::vector<std::string> COMPILER_PATHS;

    DEFINE_LITHIUM_CONSTANT(CONFIG_MANAGER)

    DEFINE_LITHIUM_CONSTANT(COMPONENT_MANAGER)
    DEFINE_LITHIUM_CONSTANT(MODULE_LOADER)
    DEFINE_LITHIUM_CONSTANT(ADDON_MANAGER)
    DEFINE_LITHIUM_CONSTANT(ENVIRONMENT)

    DEFINE_LITHIUM_CONSTANT(PROCESS_MANAGER)
    DEFINE_LITHIUM_CONSTANT(DEVICE_LOADER)
    DEFINE_LITHIUM_CONSTANT(DEVICE_MANAGER)

    DEFINE_LITHIUM_CONSTANT(THREAD_POOL)

    // QHY Compatibility
    DEFINE_LITHIUM_CONSTANT(DRIVERS_LIST)
    DEFINE_LITHIUM_CONSTANT(SYSTEM_DEVICE_LIST)
    DEFINE_LITHIUM_CONSTANT(IS_FOCUSING_LOOPING)
    DEFINE_LITHIUM_CONSTANT(MAIN_TIMER)
    DEFINE_LITHIUM_CONSTANT(MAIN_CAMERA)
    DEFINE_LITHIUM_CONSTANT(MAIN_FOCUSER)
    DEFINE_LITHIUM_CONSTANT(MAIN_FILTERWHEEL)
    DEFINE_LITHIUM_CONSTANT(MAIN_GUIDER)
    DEFINE_LITHIUM_CONSTANT(MAIN_TELESCOPE)

    DEFINE_LITHIUM_CONSTANT(TASK_CONTAINER)
    DEFINE_LITHIUM_CONSTANT(TASK_SCHEDULER)
    DEFINE_LITHIUM_CONSTANT(TASK_POOL)
    DEFINE_LITHIUM_CONSTANT(TASK_LIST)
    DEFINE_LITHIUM_CONSTANT(TASK_GENERATOR)
    DEFINE_LITHIUM_CONSTANT(TASK_MANAGER)
    DEFINE_LITHIUM_CONSTANT(TASK_QUEUE)

    DEFINE_LITHIUM_CONSTANT(SCRIPT_MANAGER)
    DEFINE_LITHIUM_CONSTANT(PYTHON_MANAGER)

    DEFINE_LITHIUM_CONSTANT(APP)
    DEFINE_LITHIUM_CONSTANT(EVENTLOOP)
    DEFINE_LITHIUM_CONSTANT(DISPATCHER)
    DEFINE_LITHIUM_CONSTANT(EXECUTOR)
    DEFINE_LITHIUM_CONSTANT(STRING_SPLITTER)
    DEFINE_LITHIUM_CONSTANT(MESSAGE_BUS)

#if ENABLE_ASYNC
    DEFINE_LITHIUM_CONSTANT(ASYNC_IO)
#endif

    static std::vector<std::string> LITHIUM_RESOURCES;
    static std::vector<std::string_view> LITHIUM_RESOURCES_SHA256;
};

#endif  // LITHIUM_UTILS_CONSTANTS_HPP
