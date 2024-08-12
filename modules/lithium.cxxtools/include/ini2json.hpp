/*
 * ini2json.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef LITHIUM_CXXTOOLS_INI2JSON_HPP
#define LITHIUM_CXXTOOLS_INI2JSON_HPP

#include <string_view>

namespace lithium::cxxtools {
auto iniToJson(std::string_view iniFilePath,
               std::string_view jsonFilePath) -> bool;
}

#endif  // LITHIUM_CXXTOOLS_INI2JSON_HPP
