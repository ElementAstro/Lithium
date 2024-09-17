/*
 * json2xml.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-7

Description: JSON to XML conversion

**************************************************/

#include "json2xml.hpp"

#include <filesystem>
#include <format>
#include <fstream>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "tinyxml2/tinyxml2.h"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace lithium::cxxtools::detail {

void jsonToXml(const json &jsonData, tinyxml2::XMLElement *xmlElement) {
    tinyxml2::XMLDocument *xmlDoc = xmlElement->GetDocument();

    for (const auto &item : jsonData.items()) {
        if (item.value().is_object()) {
            tinyxml2::XMLElement *childXmlElement =
                xmlDoc->NewElement(item.key().c_str());
            xmlElement->InsertEndChild(childXmlElement);
            jsonToXml(item.value(), childXmlElement);
        } else if (item.value().is_array()) {
            for (const auto &arrayItem : item.value()) {
                tinyxml2::XMLElement *childXmlElement =
                    xmlDoc->NewElement(item.key().c_str());
                xmlElement->InsertEndChild(childXmlElement);
                jsonToXml(arrayItem, childXmlElement);
            }
        } else if (item.value().is_string()) {
            tinyxml2::XMLElement *childXmlElement =
                xmlDoc->NewElement(item.key().c_str());
            childXmlElement->SetText(item.value().get<std::string>().c_str());
            xmlElement->InsertEndChild(childXmlElement);
        } else if (item.value().is_number()) {
            tinyxml2::XMLElement *childXmlElement =
                xmlDoc->NewElement(item.key().c_str());
            childXmlElement->SetText(
                std::to_string(item.value().get<double>()).c_str());
            xmlElement->InsertEndChild(childXmlElement);
        } else if (item.value().is_boolean()) {
            tinyxml2::XMLElement *childXmlElement =
                xmlDoc->NewElement(item.key().c_str());
            childXmlElement->SetText(item.value().get<bool>() ? "true"
                                                              : "false");
            xmlElement->InsertEndChild(childXmlElement);
        }
    }
}

auto convertJsonToXml(std::string_view jsonFilePath,
                      std::string_view xmlFilePath) -> bool {
    DLOG_F(INFO, "Reading JSON file: {}", jsonFilePath);
    if (!fs::exists(jsonFilePath) || !fs::is_regular_file(jsonFilePath)) {
        LOG_F(ERROR, "JSON file does not exist or is not a regular file: {}",
              jsonFilePath);
        return false;
    }

    std::ifstream jsonFile(jsonFilePath.data());
    if (!jsonFile.is_open()) {
        LOG_F(ERROR, "Failed to open JSON file: {}", jsonFilePath);
        return false;
    }

    json jsonData;
    try {
        jsonFile >> jsonData;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to parse JSON file: {}. Error: {}", jsonFilePath,
              e.what());
        return false;
    }
    jsonFile.close();

    tinyxml2::XMLDocument xmlDoc;
    tinyxml2::XMLElement *rootElement = xmlDoc.NewElement("root");
    xmlDoc.InsertFirstChild(rootElement);

    jsonToXml(jsonData, rootElement);

    if (xmlDoc.SaveFile(xmlFilePath.data()) != tinyxml2::XML_SUCCESS) {
        LOG_F(ERROR, "Failed to save XML file: {}", xmlFilePath);
        return false;
    }

    DLOG_F(INFO, "JSON to XML conversion succeeded.");
    return true;
}
}  // namespace lithium::cxxtools::detail

#if ATOM_STANDALONE_COMPONENT_ENABLED
#include <argparse/argparse.hpp>
int main(int argc, const char **argv) {
    loguru::init(argc, argv);
    loguru::add_file("conversion_log.txt", loguru::Append,
                     loguru::Verbosity_INFO);

    argparse::ArgumentParser program("json-to-xml");

    program.add_argument("-i", "--input")
        .required()
        .help("path to input JSON file");

    program.add_argument("-o", "--output")
        .required()
        .help("path to output XML file");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        std::cout << err.what() << std::endl;
        std::cout << program;
        return 1;
    }

    std::string jsonFilePath = program.get<std::string>("--input");
    std::string xmlFilePath = program.get<std::string>("--output");

    if (lithium::cxxtools::detail::convertJsonToXml(jsonFilePath,
                                                    xmlFilePath)) {
        DLOG_F(INFO, "JSON to XML conversion succeeded.");
    } else {
        DLOG_F(INFO, "JSON to XML conversion failed.");
    }

    return 0;
}
#else
namespace lithium::cxxtools {
auto jsonToXml(std::string_view json_file, std::string_view xml_file) -> bool {
    if (json_file.empty() || xml_file.empty()) {
        DLOG_F(ERROR, "Invalid input file path.");
        return false;
    }
    if (!fs::exists(json_file) || !fs::is_regular_file(json_file)) {
        DLOG_F(ERROR, "JSON file does not exist or is not a regular file.");
        return false;
    }
    return detail::convertJsonToXml(json_file, xml_file);
}
}  // namespace lithium::cxxtools

#endif