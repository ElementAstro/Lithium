/*
 * module.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-8-6

Description: Module Information

**************************************************/

#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <vector>


namespace lithium {
struct FunctionInfo {
    std::string name;
    void *address;
    std::vector<std::string> parameters;

    FunctionInfo() : name(""), address(nullptr) {}
};

class ModuleInfo {
    // All of the module information
public:
    std::string m_name;
    std::string m_description;
    std::string m_version;
    std::string m_status;
    std::string m_type;
    std::string m_author;
    std::string m_license;
    std::string m_path;
    std::string m_config_path;
    std::string m_config_file;

    // Module enable status
    std::atomic_bool m_enabled;

    // All of the functions in the module(dynamic loaded)
    std::vector<std::unique_ptr<FunctionInfo>> functions;

    // Module handle pointer
    void *handle;
};
}  // namespace lithium
