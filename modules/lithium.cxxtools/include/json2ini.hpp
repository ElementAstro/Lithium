/*
 * json2ini.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef LITHIUM_CXXTOOLS_JSON2INI_HPP
#define LITHIUM_CXXTOOLS_JSON2INI_HPP

#include <string_view>

namespace lithium::cxxtools {
auto jsonToIni(std::string_view jsonFilePath,
               std::string_view iniFilePath) -> bool;
}

#endif  // LITHIUM_CXXTOOLS_JSON2INI_HPP
