// ini2json.hpp
#ifndef INI2JSON_HPP
#define INI2JSON_HPP

#include "converter.hpp"

namespace lithium::cxxtools::detail {

/**
 * @brief Class for converting INI files to JSON format.
 *
 * This class provides functionality to convert INI files to JSON format
 * and save the resulting JSON data to a file. It inherits from the
 * `Converter` base class template.
 */
class Ini2Json : public Converter<Ini2Json> {
public:
    /**
     * @brief Converts an INI file to JSON format.
     *
     * This method reads the specified INI file and converts its contents
     * to JSON format.
     *
     * @param iniFilePath The path to the INI file to be converted.
     * @return nlohmann::json The converted JSON data.
     */
    nlohmann::json convertImpl(std::string_view iniFilePath);

    /**
     * @brief Saves JSON data to a specified file.
     *
     * This method saves the provided JSON data to the specified file path.
     *
     * @param jsonData The JSON data to be saved.
     * @param jsonFilePath The path to the file where the JSON data will be
     * saved.
     * @return bool True if the data was successfully saved, false otherwise.
     */
    bool saveToFileImpl(const nlohmann::json& jsonData,
                        std::string_view jsonFilePath);
};

}  // namespace lithium::cxxtools::detail

#endif  // INI2JSON_HPP