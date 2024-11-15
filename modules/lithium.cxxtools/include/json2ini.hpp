#ifndef JSON2INI_HPP
#define JSON2INI_HPP

#include "json_converter.hpp"

#include <tinyxml2.h>
#include <filesystem>

namespace lithium::cxxtools::converters {

/**
 * @brief Converter class for converting JSON to INI format.
 */
class JsonToIniConverter : public JsonConverter<JsonToIniConverter> {
public:
    /**
     * @brief Implements the conversion from JSON to INI.
     * 
     * @param jsonData The JSON data to convert.
     * @param outputPath The path to the output INI file.
     * @return true if conversion is successful, false otherwise.
     */
    bool convertImpl(const nlohmann::json& jsonData, const std::filesystem::path& outputPath);
};

} // namespace lithium::cxxtools::converters

#endif // JSON2INI_HPP