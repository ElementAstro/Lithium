/*
 * json2xml.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#ifndef JSON2XML_HPP
#define JSON2XML_HPP

#include <string_view>

namespace lithium::cxxtools {
namespace detail {
auto convertJsonToXml(std::string_view jsonFilePath,
                      std::string_view xmlFilePath) -> bool;
}  // namespace detail
auto jsonToXml(std::string_view json_file, std::string_view xml_file) -> bool;
}  // namespace lithium::cxxtools

#endif  // JSON2XML_HPP
