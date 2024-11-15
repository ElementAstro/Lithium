// yaml2json.cpp
#include "yaml2json.hpp"
#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"

using json = nlohmann::json;

namespace lithium::cxxtools::detail {

void yamlToJson(const YAML::Node& yamlNode, json& jsonData) {
    switch (yamlNode.Type()) {
        case YAML::NodeType::Null:
            jsonData = nullptr;
            break;
        case YAML::NodeType::Scalar:
            jsonData = yamlNode.as<std::string>();
            break;
        case YAML::NodeType::Sequence:
            for (const auto& item : yamlNode) {
                json jsonItem;
                yamlToJson(item, jsonItem);
                jsonData.push_back(jsonItem);
            }
            break;
        case YAML::NodeType::Map:
            for (const auto& item : yamlNode) {
                json jsonItem;
                yamlToJson(item.second, jsonItem);
                jsonData[item.first.as<std::string>()] = jsonItem;
            }
            break;
        default:
            throw std::runtime_error("Unknown YAML node type");
    }
}

json Yaml2Json::convertImpl(std::string_view yamlFilePath) {
    LOG_F(INFO, "Converting YAML file to JSON: {}", yamlFilePath);
    std::ifstream yamlFile(yamlFilePath.data());
    if (!yamlFile.is_open()) {
        THROW_RUNTIME_ERROR("Failed to open YAML file: ", yamlFilePath);
    }

    YAML::Node yamlNode = YAML::Load(yamlFile);
    json jsonData;
    yamlToJson(yamlNode, jsonData);
    return jsonData;
}

bool Yaml2Json::saveToFileImpl(const json& jsonData, std::string_view jsonFilePath) {
    LOG_F(INFO, "Saving JSON data to file: {}", jsonFilePath);
    std::ofstream jsonFile(jsonFilePath.data());
    if (!jsonFile.is_open()) {
        LOG_F(ERROR, "Failed to open JSON file: {}", jsonFilePath);
        return false;
    }
    jsonFile << std::setw(4) << jsonData << std::endl;
    return true;
}

} // namespace lithium::cxxtools::detail