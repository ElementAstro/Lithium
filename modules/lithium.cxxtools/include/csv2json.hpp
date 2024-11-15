// csv2json.hpp
#ifndef CSV2JSON_HPP
#define CSV2JSON_HPP

#include "converter.hpp"

namespace lithium::cxxtools::detail {

/**
 * @brief Class for converting CSV files to JSON format.
 * 
 * This class provides functionality to convert CSV files to JSON format
 * and save the resulting JSON data to a file. It inherits from the
 * `Converter` base class template.
 */
class Csv2Json : public Converter<Csv2Json> {
public:
    /**
     * @brief Converts a CSV file to JSON format.
     * 
     * This method reads the specified CSV file and converts its contents
     * to JSON format.
     * 
     * @param csvFilePath The path to the CSV file to be converted.
     * @return nlohmann::json The converted JSON data.
     */
    auto convertImpl(std::string_view csvFilePath) -> nlohmann::json;

    /**
     * @brief Saves JSON data to a specified file.
     * 
     * This method saves the provided JSON data to the specified file path.
     * 
     * @param jsonData The JSON data to be saved.
     * @param jsonFilePath The path to the file where the JSON data will be saved.
     * @return bool True if the data was successfully saved, false otherwise.
     */
    bool saveToFileImpl(const nlohmann::json& jsonData, std::string_view jsonFilePath);
};

} // namespace lithium::cxxtools::detail

#endif // CSV2JSON_HPP