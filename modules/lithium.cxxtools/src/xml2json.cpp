/*
 * xml2json.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-7

Description: XML to JSON conversion

**************************************************/

#include "xml2json.hpp"

#include <fstream>
#include <iomanip>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "tinyxml2/tinyxml2.h"

using json = nlohmann::json;

namespace lithium::cxxtools::detail {
void xmlToJson(const tinyxml2::XMLElement *xmlElement, json &jsonData) {
    for (const tinyxml2::XMLNode *childNode = xmlElement->FirstChild();
         childNode != nullptr; childNode = childNode->NextSibling()) {
        if (childNode->ToElement() != nullptr) {
            const std::string CHILD_NODE_NAME = childNode->Value();
            json &jsonChildValue = jsonData[CHILD_NODE_NAME];
            if (!jsonChildValue.is_null()) {
                if (!jsonChildValue.is_array()) {
                    jsonChildValue = json::array({jsonChildValue});
                }
            } else {
                jsonChildValue = json::array();
            }

            json jsonItemValue;
            xmlToJson(childNode->ToElement(), jsonItemValue);
            jsonChildValue.push_back(jsonItemValue);
        } else if (childNode->ToText() != nullptr) {
            jsonData = json(childNode->ToText()->Value());
        }
    }
}

auto convertXmlToJson(std::string_view xmlFilePath,
                      std::string_view jsonFilePath) -> bool {
    DLOG_F(INFO, "Reading XML file: {}", xmlFilePath);
    tinyxml2::XMLDocument xmlDoc;
    if (xmlDoc.LoadFile(xmlFilePath.data()) != tinyxml2::XML_SUCCESS) {
        DLOG_F(ERROR, "Failed to load XML file: {}", xmlFilePath);
        return false;
    }

    json jsonData;
    DLOG_F(INFO, "Converting XML to JSON");
    xmlToJson(xmlDoc.RootElement(), jsonData);

    DLOG_F(INFO, "Saving JSON file: {}", jsonFilePath);
    std::ofstream jsonFile(jsonFilePath.data());
    if (!jsonFile.is_open()) {
        DLOG_F(ERROR, "Failed to open JSON file: {}", jsonFilePath);
        return false;
    }

    jsonFile << std::setw(4) << jsonData << std::endl;
    jsonFile.close();

    DLOG_F(INFO, "XML to JSON conversion succeeded.");
    return true;
}
}  // namespace lithium::cxxtools::detail

#if ATOM_STANDALONE_COMPONENT_ENABLED
#include <argparse/argparse.hpp>
int main(int argc, char *argv[]) {
    loguru::init(argc, argv);
    loguru::add_file("conversion_log.txt", loguru::Append,
                     loguru::Verbosity_INFO);
    argparse::ArgumentParser program("xml-to-json");

    program.add_argument("-i", "--input")
        .required()
        .help("path to input XML file");
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

    std::string xmlFilePath = program.get<std::string>("--input");
    std::string jsonFilePath = program.get<std::string>("--output");

    if (convertXmlToJson(xmlFilePath, jsonFilePath)) {
        DLOG_F(INFO, "XML to JSON conversion succeeded.");
    } else {
        DLOG_F(INFO, "XML to JSON conversion failed.");
    }

    return 0;
}
#else
namespace lithium::cxxtools {
auto xmlToJson(std::string_view xml_file, std::string_view json_file) -> bool {
    try {
        if (detail::convertXmlToJson(xml_file, json_file)) {
            DLOG_F(INFO, "XML to JSON conversion succeeded.");
            return true;
        }
    } catch (const std::exception &e) {
        DLOG_F(ERROR, "Conversion failed: {}", e.what());
    }
    DLOG_F(INFO, "XML to JSON conversion failed.");
    return false;
}
}  // namespace lithium::cxxtools

#endif
