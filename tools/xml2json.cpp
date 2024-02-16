/*
 * xml2json.cpp
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

Description: XML to JSON conversion

**************************************************/

#include <fstream>
#include <string>
#include "atom/type/json.hpp"
#include "tinyxml2/tinyxml2.h"
#include "atom/log/loguru.hpp"
#include <argparse/argparse.hpp>

using json = nlohmann::json;

void xmlToJson(const tinyxml2::XMLElement *xmlElement, json &jsonData)
{
    for (const tinyxml2::XMLNode *childNode = xmlElement->FirstChild(); childNode != nullptr; childNode = childNode->NextSibling())
    {
        if (childNode->ToElement())
        {
            const std::string childNodeName = childNode->Value();
            json &jsonChildValue = jsonData[childNodeName];
            if (!jsonChildValue.is_null())
            {
                if (!jsonChildValue.is_array())
                {
                    jsonChildValue = json::array();
                    jsonChildValue.push_back(jsonData[childNodeName]);
                }
            }
            else
            {
                jsonChildValue = json::array();
            }

            json jsonItemValue;
            xmlToJson(childNode->ToElement(), jsonItemValue);
            jsonChildValue.push_back(jsonItemValue);
        }
        else if (childNode->ToText())
        {
            jsonData = json(childNode->ToText()->Value());
        }
    }
}

bool convertXmlToJson(const std::string &xmlFilePath, const std::string &jsonFilePath)
{
    // 设置 loguru 的日志文件
    loguru::add_file("conversion.log", loguru::Append, loguru::Verbosity_INFO);

    // 读取 XML 文件
    DLOG_F(INFO, "Reading XML file: {}", xmlFilePath);
    tinyxml2::XMLDocument xmlDoc;
    if (xmlDoc.LoadFile(xmlFilePath.c_str()) != tinyxml2::XML_SUCCESS)
    {
        DLOG_F(ERROR, "Failed to load XML file: {}", xmlFilePath);
        return false;
    }

    // 创建 JSON 对象
    json jsonData;

    // 转换 XML 到 JSON
    DLOG_F(INFO, "Converting XML to JSON");
    xmlToJson(xmlDoc.RootElement(), jsonData);

    // 保存 JSON 对象到文件
    DLOG_F(INFO, "Saving JSON file: {}", jsonFilePath);
    std::ofstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open())
    {
        DLOG_F(ERROR, "Failed to open JSON file: {}", jsonFilePath);
        return false;
    }

    jsonFile << std::setw(4) << jsonData << std::endl;
    jsonFile.close();

    DLOG_F(INFO, "XML to JSON conversion succeeded.");
    return true;
}

int main(int argc, const char **argv)
{
    argparse::ArgumentParser program("xml-to-json");

    program.add_argument("-i", "--input")
        .required()
        .help("path to input XML file");

    program.add_argument("-o", "--output")
        .required()
        .help("path to output JSON file");

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

    std::string xmlFilePath = program.get<std::string>("--input");
    std::string jsonFilePath = program.get<std::string>("--output");

    if (convertXmlToJson(xmlFilePath, jsonFilePath))
    {
        DLOG_F(INFO, "XML to JSON conversion succeeded.");
    }
    else
    {
        DLOG_F(INFO, "XML to JSON conversion failed.");
    }

    return 0;
}
