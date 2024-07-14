/*
 * xml2json.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef LITHIUM_CXXTOOLS_XML2JSON_HPP
#define LITHIUM_CXXTOOLS_XML2JSON_HPP

#include <string_view>

namespace lithium::cxxtools {
namespace detail {
/**
 * @brief Convert XML file to JSON file
 *
 * @param xmlFilePath Path to the XML file
 * @param jsonFilePath Path to the JSON file
 * @return true if conversion was successful
 * @return false if conversion failed
 */
auto convertXmlToJson(std::string_view xmlFilePath,
                      std::string_view jsonFilePath) -> bool;
}  // namespace detail
/**
 * @brief Convert XML file to JSON file
 *
 * @param xml_file Path to the XML file
 * @param json_file Path to the JSON file
 * @return true if conversion was successful
 * @return false if conversion failed
 */
auto xmlToJson(std::string_view xml_file, std::string_view json_file) -> bool;
}  // namespace lithium::cxxtools

#endif  // LITHIUM_CXXTOOLS_XML2JSON_HPP
