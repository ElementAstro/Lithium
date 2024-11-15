#ifndef JSON2CSV_HPP
#define JSON2CSV_HPP

#include "json_converter.hpp"

#include <filesystem>

namespace lithium::cxxtools::converters {

/**
 * @brief Converter class for converting JSON to CSV format.
 */
class JsonToCsvConverter : public JsonConverter<JsonToCsvConverter> {
public:
    /**
     * @brief Implements the conversion from JSON to CSV.
     * 
     * @param jsonData The JSON data to convert.
     * @param outputPath The path to the output CSV file.
     * @return true if conversion is successful, false otherwise.
     */
    bool convertImpl(const nlohmann::json& jsonData, const std::filesystem::path& outputPath);
};

} // namespace lithium::cxxtools::converters

#endif // JSON2CSV_HPP