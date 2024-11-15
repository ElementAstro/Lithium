#ifndef JSON2XML_HPP
#define JSON2XML_HPP

#include "json_converter.hpp"

#include <tinyxml2.h>
#include <filesystem>

namespace lithium::cxxtools::converters {

/**
 * @brief Converter class for converting JSON to XML format.
 */
class JsonToXmlConverter : public JsonConverter<JsonToXmlConverter> {
public:
    /**
     * @brief Implements the conversion from JSON to XML.
     * 
     * @param jsonData The JSON data to convert.
     * @param outputPath The path to the output XML file.
     * @return true if conversion is successful, false otherwise.
     */
    bool convertImpl(const nlohmann::json& jsonData, const std::filesystem::path& outputPath);
};

} // namespace lithium::cxxtools::converters

#endif // JSON2XML_HPP