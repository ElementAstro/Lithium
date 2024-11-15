// converter.hpp
#ifndef CONVERTER_HPP
#define CONVERTER_HPP

#include <string_view>
#include "atom/type/json.hpp"

namespace lithium::cxxtools {

/**
 * @brief Base class template for file format converters using static
 * polymorphism.
 *
 * This class provides a common interface for converting files to JSON format
 * and saving JSON data to files. Derived classes must implement the
 * `convertImpl` and `saveToFileImpl` methods to provide specific conversion
 * logic.
 *
 * @tparam Derived The derived converter class.
 */
template <typename Derived>
class Converter {
public:
    /**
     * @brief Converts the input file to JSON format.
     *
     * This method delegates the conversion to the derived class by calling
     * its `convertImpl` method.
     *
     * @param inputFilePath The path to the input file to be converted.
     * @return nlohmann::json The converted JSON data.
     */
    nlohmann::json convert(std::string_view inputFilePath) {
        return static_cast<Derived*>(this)->convertImpl(inputFilePath);
    }

    /**
     * @brief Saves the JSON data to the specified output file.
     *
     * This method delegates the saving operation to the derived class by
     * calling its `saveToFileImpl` method.
     *
     * @param jsonData The JSON data to be saved.
     * @param outputFilePath The path to the output file where the JSON data
     * will be saved.
     * @return bool True if the data was successfully saved, false otherwise.
     */
    bool saveToFile(const nlohmann::json& jsonData,
                    std::string_view outputFilePath) {
        return static_cast<Derived*>(this)->saveToFileImpl(jsonData,
                                                           outputFilePath);
    }
};

}  // namespace lithium::cxxtools

#endif  // CONVERTER_HPP