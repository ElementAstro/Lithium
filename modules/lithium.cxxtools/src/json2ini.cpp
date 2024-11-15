#include "json2ini.hpp"

#include <format>
#include <fstream>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace lithium::cxxtools::converters {

/**
 * @brief Writes a section to the INI file.
 *
 * @param iniFile The output INI file stream.
 * @param sectionName The name of the INI section.
 * @param jsonObject The JSON object containing key-value pairs.
 */
void writeIniSection(std::ofstream& iniFile, std::string_view sectionName,
                     const nlohmann::json& jsonObject) {
    iniFile << "[" << sectionName << "]" << std::endl;
    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
        if (it->is_string()) {
            iniFile << std::format("{}={}\n", it.key(), it->get<std::string>());
        } else if (it->is_number()) {
            iniFile << std::format("{}={}\n", it.key(), it->get<double>());
        } else if (it->is_boolean()) {
            iniFile << std::format("{}={}\n", it.key(),
                                   it->get<bool>() ? "true" : "false");
        } else {
            LOG_F(WARNING, "Unsupported JSON type for key '{}'", it.key());
            iniFile << std::format("{}={}\n", it.key(), "null");
        }
    }
    iniFile << std::endl;
}

bool JsonToIniConverter::convertImpl(const nlohmann::json& jsonData,
                                     const std::filesystem::path& outputPath) {
    LOG_F(INFO, "Starting JSON to INI conversion.");

    std::ofstream iniFile(outputPath);
    if (!iniFile.is_open()) {
        LOG_F(ERROR, "Failed to open INI file for writing: {}",
              outputPath.string());
        THROW_FAIL_TO_OPEN_FILE("Failed to open INI file for writing: {}",
                                outputPath.string());
    }

    for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
        if (it->is_object()) {
            writeIniSection(iniFile, it.key(), *it);
        } else {
            LOG_F(WARNING, "Skipping non-object JSON element with key '{}'",
                  it.key());
        }
    }

    if (!iniFile) {
        LOG_F(ERROR, "Failed to write to INI file: {}", outputPath.string());
        THROW_FILE_NOT_WRITABLE("Failed to write to INI file: {}",
                                outputPath.string());
    }

    LOG_F(INFO, "Successfully converted JSON to INI: {}", outputPath.string());
    return true;
}

}  // namespace lithium::cxxtools::converters