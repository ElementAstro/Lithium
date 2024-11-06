/*
 * yaml2json.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-7

Description: YAML to JSON conversion

**************************************************/

#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include <yaml-cpp/yaml.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace lithium::cxxtools::detail {
void yamlToJson(const YAML::Node &yamlNode, json &jsonData) {
    switch (yamlNode.Type()) {
        case YAML::NodeType::Null:
            jsonData = nullptr;
            break;
        case YAML::NodeType::Scalar:
            jsonData = yamlNode.as<std::string>();
            break;
        case YAML::NodeType::Sequence:
            for (const auto &item : yamlNode) {
                json jsonItem;
                yamlToJson(item, jsonItem);
                jsonData.push_back(jsonItem);
            }
            break;
        case YAML::NodeType::Map:
            for (const auto &item : yamlNode) {
                json jsonItem;
                yamlToJson(item.second, jsonItem);
                jsonData[item.first.as<std::string>()] = jsonItem;
            }
            break;
        default:
            throw std::runtime_error("Unknown YAML node type");
    }
}

auto convertYamlToJson(std::string_view yamlFilePath,
                       std::string_view jsonFilePath) -> bool {
    std::ifstream yamlFile(yamlFilePath.data());
    if (!yamlFile.is_open()) {
        std::cerr << "Failed to open YAML file: " << yamlFilePath << std::endl;
        return false;
    }

    YAML::Node yamlNode = YAML::Load(yamlFile);
    json jsonData;
    yamlToJson(yamlNode, jsonData);

    std::ofstream jsonFile(jsonFilePath.data());
    if (!jsonFile.is_open()) {
        std::cerr << "Failed to open JSON file: " << jsonFilePath << std::endl;
        return false;
    }

    jsonFile << std::setw(4) << jsonData << std::endl;
    jsonFile.close();

    std::cout << "YAML to JSON conversion succeeded." << std::endl;
    return true;
}

}  // namespace lithium::cxxtools::detail

#if ATOM_STANDALONE_COMPONENT_ENABLED
#include <argparse/argparse.hpp>
int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("yaml-to-json");

    program.add_argument("-i", "--input")
        .required()
        .help("path to input YAML file");
    program.add_argument("-o", "--output")
        .required()
        .help("path to output JSON file");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        return 1;
    }

    std::string yamlFilePath = program.get<std::string>("--input");
    std::string jsonFilePath = program.get<std::string>("--output");

    if (lithium::cxxtools::detail::convertYamlToJson(yamlFilePath,
                                                     jsonFilePath)) {
        std::cout << "YAML to JSON conversion succeeded." << std::endl;
    } else {
        std::cout << "YAML to JSON conversion failed." << std::endl;
    }

    return 0;
}
#else
namespace lithium::cxxtools {
auto yamlToJson(std::string_view yaml_file,
                std::string_view json_file) -> bool {
    try {
        if (detail::convertYamlToJson(yaml_file, json_file)) {
            std::cout << "YAML to JSON conversion succeeded." << std::endl;
            return true;
        }
    } catch (const std::exception &e) {
        std::cerr << "Conversion failed: " << e.what() << std::endl;
    }
    std::cout << "YAML to JSON conversion failed." << std::endl;
    return false;
}
}  // namespace lithium::cxxtools

#endif
