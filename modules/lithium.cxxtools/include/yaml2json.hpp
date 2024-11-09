/*
 * yaml2json.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef LITHIUM_CXXTOOLS_YAML2JSON_HPP
#define LITHIUM_CXXTOOLS_YAML2JSON_HPP

#include <string_view>

namespace lithium::cxxtools {
/**
 * @brief Convert YAML file to JSON file
 *
 * @param yamlFilePath Path to the YAML file
 * @param jsonFilePath Path to the JSON file
 * @return true if conversion was successful
 * @return false if conversion failed
 */
auto yamlToJson(std::string_view yaml_file, std::string_view json_file) -> bool;
}  // namespace lithium::cxxtools

#endif  // LITHIUM_CXXTOOLS_YAML2JSON_HPP
