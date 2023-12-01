#include <iostream>
#include <fstream>
#include <string>
#include "nlohmann/json.hpp"
#include "pugixml/pugixml.hpp"
#include "atom/log/loguru.hpp"
#include <argparse/argparse.hpp>

using json = nlohmann::json;

void xmlToJson(const pugi::xml_node &xmlNode, json &jsonData)
{
    for (const auto &childXmlNode : xmlNode.children())
    {
        if (childXmlNode.type() == pugi::node_element)
        {
            const std::string childNodeName = childXmlNode.name();
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
            xmlToJson(childXmlNode, jsonItemValue);
            jsonChildValue.push_back(jsonItemValue);
        }
        else if (childXmlNode.type() == pugi::node_pcdata)
        {
            jsonData = json(childXmlNode.value());
        }
    }
}

bool convertXmlToJson(const std::string &xmlFilePath, const std::string &jsonFilePath)
{
    // 设置 loguru 的日志文件
    loguru::add_file("conversion.log", loguru::Append, loguru::Verbosity_INFO);

    // 读取 XML 文件
    DLOG_F(INFO, "Reading XML file: %s", xmlFilePath.c_str());
    pugi::xml_document xmlDoc;
    if (!xmlDoc.load_file(xmlFilePath.c_str()))
    {
        DLOG_F(ERROR, "Failed to load XML file: %s", xmlFilePath.c_str());
        return false;
    }

    // 创建 JSON 对象
    json jsonData;

    // 转换 XML 到 JSON
    DLOG_F(INFO, "Converting XML to JSON");
    xmlToJson(xmlDoc, jsonData);

    // 保存 JSON 对象到文件
    DLOG_F(INFO, "Saving JSON file: %s", jsonFilePath.c_str());
    std::ofstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open())
    {
        DLOG_F(ERROR, "Failed to open JSON file: %s", jsonFilePath.c_str());
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
