#ifndef LITHIUM_CXXTOOLS_CSV2JSON_HPP
#define LITHIUM_CXXTOOLS_CSV2JSON_HPP

#include <string_view>

#include "atom/type/json_fwd.hpp"
using json = nlohmann::json;

namespace lithium::cxxtools {
namespace detail {
/**
 * @brief Convert a CSV file to a JSON object
 * @param csvFilePath The path to the CSV file
 * @return The JSON object
 */
auto csvToJson(std::string_view csvFilePath) -> json;

/**
 * @brief Save a JSON object to a file
 * @param jsonData The JSON object
 * @param jsonFilePath The path to the JSON file
 */
void saveJsonToFile(const json &jsonData, std::string_view jsonFilePath);
}  // namespace detail
/**
 * @brief Convert a CSV file to a JSON file
 * @param csv_file The path to the CSV file
 * @param json_file The path to the JSON file
 * @return true if the conversion was successful
 */
auto csvToJson(std::string_view csv_file, std::string_view json_file) -> bool;
}  // namespace lithium::cxxtools

#endif
