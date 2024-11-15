#ifndef JSON2YAML_HPP
#define JSON2YAML_HPP

#include "json_converter.hpp"

#include <filesystem>

namespace lithium::cxxtools::converters {

/**
 * @brief Converter class for converting JSON to YAML format.
 */
class JsonToYamlConverter : public JsonConverter<JsonToYamlConverter> {
public:
    /**
     * @brief Implements the conversion from JSON to YAML.
     * 
     * @param jsonData The JSON data to convert.
     * @param outputPath The path to the output YAML file.
     * @return true if conversion is successful, false otherwise.
     */
    bool convertImpl(const nlohmann::json& jsonData, const std::filesystem::path& outputPath);
};

} // namespace lithium::cxxtools::converters

#endif // JSON2YAML_HPP