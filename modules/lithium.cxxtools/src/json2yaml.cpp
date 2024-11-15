#include "json2yaml.hpp"

#include <yaml-cpp/yaml.h>
#include <fstream>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace lithium::cxxtools::converters {

/**
 * @brief Recursively converts JSON data to YAML node.
 *
 * @param out The YAML emitter.
 * @param jsonData The JSON data.
 */
void jsonToYaml(YAML::Emitter& out, const nlohmann::json& jsonData) {
    if (jsonData.is_object()) {
        for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
            out << YAML::Key << it.key() << YAML::Value;
            jsonToYaml(out, *it);
        }
    } else if (jsonData.is_array()) {
        out << YAML::BeginSeq;
        for (const auto& item : jsonData) {
            jsonToYaml(out, item);
        }
        out << YAML::EndSeq;
    } else if (jsonData.is_string()) {
        out << jsonData.get<std::string>();
    } else if (jsonData.is_number()) {
        out << jsonData.get<double>();
    } else if (jsonData.is_boolean()) {
        out << (jsonData.get<bool>() ? "true" : "false");
    } else {
        LOG_F(WARNING, "Encountered unsupported JSON type during conversion.");
        out << "null";
    }
}

bool JsonToYamlConverter::convertImpl(const nlohmann::json& jsonData,
                                      const std::filesystem::path& outputPath) {
    LOG_F(INFO, "Starting JSON to YAML conversion.");

    YAML::Emitter out;
    out << YAML::BeginMap;
    try {
        jsonToYaml(out, jsonData);
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception during JSON to YAML conversion: {}", e.what());
        THROW_RUNTIME_ERROR("Exception during JSON to YAML conversion: {}",
                            e.what());
    }
    out << YAML::EndMap;

    std::ofstream yamlFile(outputPath);
    if (!yamlFile.is_open()) {
        LOG_F(ERROR, "Failed to open YAML file for writing: {}",
              outputPath.string());
        THROW_FAIL_TO_OPEN_FILE("Failed to open YAML file for writing: {}",
                                outputPath.string());
    }

    yamlFile << out.c_str();
    if (!yamlFile) {
        LOG_F(ERROR, "Failed to write to YAML file: {}", outputPath.string());
        THROW_FILE_NOT_WRITABLE("Failed to write to YAML file: {}",
                                outputPath.string());
    }

    LOG_F(INFO, "Successfully converted JSON to YAML: {}", outputPath.string());
    return true;
}

}  // namespace lithium::cxxtools::converters