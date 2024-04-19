/*
 * resource.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-16

Description: Resource List for Lithium

**************************************************/

#ifndef LITHIUM_UTILS_RESOURCE_HPP
#define LITHIUM_UTILS_RESOURCE_HPP

#include <string>
#include <unordered_map>

class resource {
public:
    static std::unordered_map<std::string_view,
                                   std::pair<std::string, bool>>
        LITHIUM_RESOURCES;

    static constexpr const char* LITHIUM_RESOURCE_SERVER =
        "https://github/ElementAstro/LithiumPackage";
};

std::unordered_map<std::string_view, std::pair<std::string, bool>>
    resource::LITHIUM_RESOURCES = {{
#ifdef _WIN32
        "lithium_server.exe"
#else
        "lithium_server"
#endif
        ,
        {"", false}}};

#endif