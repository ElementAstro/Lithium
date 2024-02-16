/*
 * json2xml.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Date: 2023-12-7

Description: Json to XML conversion

**************************************************/

#include <fstream>
#include <string>
#include "atom/type/json.hpp"
#include "tinyxml2/tinyxml2.h"
#include "atom/log/loguru.hpp"
#include <argparse/argparse.hpp>

using json = nlohmann::json;

void jsonToXml(const json &jsonData, tinyxml2::XMLElement *xmlElement)
{
    tinyxml2::XMLDocument *xmlDoc = xmlElement->GetDocument();

    for (const auto &item : jsonData.items())
    {
        if (item.value().is_object())
        {
            tinyxml2::XMLElement *childXmlElement = xmlDoc->NewElement(item.key().c_str());
            xmlElement->InsertEndChild(childXmlElement);
            jsonToXml(item.value(), childXmlElement);
        }
        else if (item.value().is_array())
        {
            for (const auto &arrayItem : item.value())
            {
                tinyxml2::XMLElement *childXmlElement = xmlDoc->NewElement(item.key().c_str());
                xmlElement->InsertEndChild(childXmlElement);
                jsonToXml(arrayItem, childXmlElement);
            }
        }
        else
        {
            tinyxml2::XMLElement *childXmlElement = xmlDoc->NewElement(item.key().c_str());
            childXmlElement->SetText(item.value().get<std::string>().c_str());
            xmlElement->InsertEndChild(childXmlElement);
        }
    }
}

bool convertJsonToXml(const std::string &jsonFilePath, const std::string &xmlFilePath)
{
    loguru::add_file("conversion.log", loguru::Append, loguru::Verbosity_INFO);

    DLOG_F(INFO, "Reading JSON file: {}", jsonFilePath);
    // 读取 JSON 文件
    std::ifstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open())
    {
        std::cout << "Failed to open JSON file: " << jsonFilePath << std::endl;
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
    if (xmlDoc.SaveFile(xmlFilePath.c_str()) != tinyxml2::XML_SUCCESS)
    {
        LOG_F(ERROR, "Failed to save XML file: {}", xmlFilePath);
        return false;
    }

    DLOG_F(INFO, "JSON to XML conversion succeeded.");
    return true;
}

int main(int argc, const char **argv)
{
    argparse::ArgumentParser program("json-to-xml");

    program.add_argument("-i", "--input")
        .required()
        .help("path to input JSON file");

    program.add_argument("-o", "--output")
        .required()
        .help("path to output XML file");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        std::cout << err.what() << std::endl;
        std::cout << program;
        return 1;
    }

    std::string jsonFilePath = program.get<std::string>("--input");
    std::string xmlFilePath = program.get<std::string>("--output");

    if (convertJsonToXml(jsonFilePath, xmlFilePath))
    {
        DLOG_F(INFO, "JSON to XML conversion succeeded.");
    }
    else
    {
        DLOG_F(INFO, "JSON to XML conversion failed.");
    }

    return 0;
}
