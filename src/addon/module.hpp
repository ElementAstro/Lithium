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

#include "atom/function/ffi.hpp"
#include "macro.hpp"

namespace lithium {
struct FunctionInfo {
    std::string name;
    void* address;
    std::vector<std::string> parameters;

    FunctionInfo() : name(""), address(nullptr) {}
};

struct ModuleInfo {
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

    std::shared_ptr<atom::meta::DynamicLibrary> mLibrary;
} ATOM_ALIGNAS(128);

}  // namespace lithium
