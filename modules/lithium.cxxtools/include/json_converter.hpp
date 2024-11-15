#ifndef JSON_CONVERTER_HPP
#define JSON_CONVERTER_HPP

#include <filesystem>

#include "atom/type/json_fwd.hpp"

/**
 * @brief Base class template for JSON converters using static polymorphism.
 *
 * @tparam Derived The derived converter class.
 */
template <typename Derived>
class JsonConverter {
public:
    /**
     * @brief Converts JSON data to the desired format.
     *
     * @param jsonData The JSON data to convert.
     * @param outputPath The path to the output file.
     * @return true if conversion is successful, false otherwise.
     */
    bool convert(const nlohmann::json& jsonData,
                 const std::filesystem::path& outputPath) {
        return static_cast<Derived*>(this)->convertImpl(jsonData, outputPath);
    }
};

#endif  // JSON_CONVERTER_HPP