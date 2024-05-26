/*
 * json2xml.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-7

Description: Json to XML conversion

**************************************************/

#include "json2xml.hpp"

#include <fstream>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "tinyxml2/tinyxml2.h"

using json = nlohmann::json;

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
        } else {
            tinyxml2::XMLElement *childXmlElement =
                xmlDoc->NewElement(item.key().c_str());
            childXmlElement->SetText(item.value().get<std::string>().c_str());
            xmlElement->InsertEndChild(childXmlElement);
        }
    }
}

bool convertJsonToXml(const std::string &jsonFilePath,
                      const std::string &xmlFilePath) {
    DLOG_F(INFO, "Reading JSON file: {}", jsonFilePath);
    // 读取 JSON 文件
    std::ifstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open()) {
        LOG_F(ERROR, "Failed to open JSON file: {}", jsonFilePath);
        return false;
    }

    // 解析 JSON
    json jsonData;
    jsonFile >> jsonData;
    jsonFile.close();

    // 创建 XML 文档
    tinyxml2::XMLDocument xmlDoc;
    tinyxml2::XMLElement *rootElement = xmlDoc.NewElement("root");
    xmlDoc.InsertFirstChild(rootElement);

    // 转换 JSON 到 XML
    jsonToXml(jsonData, rootElement);

    // 保存 XML 文档到文件
    if (xmlDoc.SaveFile(xmlFilePath.c_str()) != tinyxml2::XML_SUCCESS) {
        LOG_F(ERROR, "Failed to save XML file: {}", xmlFilePath);
        return false;
    }

    DLOG_F(INFO, "JSON to XML conversion succeeded.");
    return true;
}

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

    if (convertJsonToXml(jsonFilePath, xmlFilePath)) {
        DLOG_F(INFO, "JSON to XML conversion succeeded.");
    } else {
        DLOG_F(INFO, "JSON to XML conversion failed.");
    }

    return 0;
}
#else
bool json_to_xml(const std::string &json_file, const std::string &xml_file) {
    if (json_file.empty() || xml_file.empty()) {
        DLOG_F(ERROR, "Invalid input file path.");
        return false;
    }
    if (!std::filesystem::exists(json_file) ||
        !std::filesystem::is_regular_file(json_file)) {
        DLOG_F(ERROR, "Json file does not exist or is not a regular file.");
        return false;
    }
    if (convertJsonToXml(json_file, xml_file)) {
        DLOG_F(INFO, "JSON to XML conversion succeeded.");
        return true;
    }
    DLOG_F(INFO, "JSON to XML conversion failed.");

    return false;
}
#endif