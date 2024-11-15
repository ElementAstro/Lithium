#include "json2csv.hpp"

#include <format>
#include <fstream>
#include <unordered_set>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace lithium::cxxtools::converters {

/**
 * @brief Flattens JSON objects into a single-level map for CSV compatibility.
 *
 * @param jsonData The JSON data to flatten.
 * @param parentKey The prefix for keys (used in recursion).
 * @param flatMap The resulting flattened map.
 */
void flattenJson(const nlohmann::json& jsonData, const std::string& parentKey,
                 std::unordered_map<std::string, std::string>& flatMap) {
    if (jsonData.is_object()) {
        for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
            std::string newKey =
                parentKey.empty() ? it.key() : parentKey + "_" + it.key();
            flattenJson(it.value(), newKey, flatMap);
        }
    } else if (jsonData.is_array()) {
        int index = 0;
        for (const auto& item : jsonData) {
            std::string newKey = parentKey + "_" + std::to_string(index++);
            flattenJson(item, newKey, flatMap);
        }
    } else {
        flatMap[parentKey] = jsonData.dump();
    }
}

bool JsonToCsvConverter::convertImpl(const nlohmann::json& jsonData,
                                     const std::filesystem::path& outputPath) {
    LOG_F(INFO, "Starting JSON to CSV conversion.");

    if (!jsonData.is_array()) {
        LOG_F(
            ERROR,
            "JSON data is not an array. CSV conversion requires a JSON array.");
        THROW_RUNTIME_ERROR(
            "JSON data is not an array. CSV conversion requires a JSON array.");
    }

    std::ofstream csvFile(outputPath);
    if (!csvFile.is_open()) {
        LOG_F(ERROR, "Failed to open CSV file for writing: {}",
              outputPath.string());
        THROW_FAIL_TO_OPEN_FILE("Failed to open CSV file for writing: {}",
                                outputPath.string());
    }

    // Determine headers
    std::vector<std::string> headers;
    std::vector<std::unordered_map<std::string, std::string>> flatData;
    std::unordered_set<std::string> headerSet;
    for (const auto& item : jsonData) {
        std::unordered_map<std::string, std::string> flatMap;
        flattenJson(item, "", flatMap);
        for (const auto& [key, _] : flatMap) {
            if (headerSet.insert(key).second) {
                headers.push_back(key);
            }
        }
        flatData.push_back(flatMap);
    }

    // Write headers
    for (size_t i = 0; i < headers.size(); ++i) {
        csvFile << "\"" << headers[i] << "\"";
        if (i != headers.size() - 1) {
            csvFile << ",";
        }
    }
    csvFile << "\n";

    // Write data rows
    for (const auto& flatMap : flatData) {
        for (size_t i = 0; i < headers.size(); ++i) {
            auto it = flatMap.find(headers[i]);
            if (it != flatMap.end()) {
                std::string value = it->second;
                // Escape quotes by doubling them
                size_t pos = 0;
                while ((pos = value.find('"', pos)) != std::string::npos) {
                    value.insert(pos, 1, '"');
                    pos += 2;
                }
                csvFile << "\"" << value << "\"";
            } else {
                csvFile << "\"\"";
            }
            if (i != headers.size() - 1) {
                csvFile << ",";
            }
        }
        csvFile << "\n";
    }

    if (!csvFile) {
        LOG_F(ERROR, "Failed to write to CSV file: {}", outputPath.string());
        THROW_FILE_NOT_WRITABLE("Failed to write to CSV file: {}",
                                outputPath.string());
    }

    LOG_F(INFO, "Successfully converted JSON to CSV: {}", outputPath.string());
    return true;
}

}  // namespace lithium::cxxtools::converters