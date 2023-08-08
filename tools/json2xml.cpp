#include <iostream>
#include <fstream>
#include <string>
#include "nlohmann/json.hpp"
#include "pugixml/pugixml.hpp"
#include "loguru/loguru.hpp"
#include <argparse/argparse.hpp>

using json = nlohmann::json;

void jsonToXml(const json &jsonData, pugi::xml_node &xmlNode)
{
    for (const auto &item : jsonData.items())
    {
        if (item.value().is_object())
        {
            pugi::xml_node childXmlNode = xmlNode.append_child(item.key().c_str());
            jsonToXml(item.value(), childXmlNode);
        }
        else if (item.value().is_array())
        {
            for (const auto &arrayItem : item.value())
            {
                pugi::xml_node childXmlNode = xmlNode.append_child(item.key().c_str());
                jsonToXml(arrayItem, childXmlNode);
            }
        }
        else
        {
            xmlNode.append_child(item.key().c_str()).text().set(item.value().get<std::string>().c_str());
        }
    }
}

bool convertJsonToXml(const std::string &jsonFilePath, const std::string &xmlFilePath)
{
    // 设置 loguru 的日志文件
    loguru::add_file("conversion.log", loguru::Append, loguru::Verbosity_INFO);

    // 读取 JSON 文件
    LOG_F(INFO, "Reading JSON file: %s", jsonFilePath.c_str());
    std::ifstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open())
    {
        LOG_F(ERROR, "Failed to open JSON file: %s", jsonFilePath.c_str());
        return false;
    }

    // 解析 JSON
    LOG_F(INFO, "Parsing JSON data");
    json jsonData;
    jsonFile >> jsonData;
    jsonFile.close();

    // 创建 XML 文档
    LOG_F(INFO, "Creating XML document");
    pugi::xml_document xmlDoc;

    // 转换 JSON 到 XML
    LOG_F(INFO, "Converting JSON to XML");
    jsonToXml(jsonData, xmlDoc);

    // 保存 XML 文档到文件
    LOG_F(INFO, "Saving XML file: %s", xmlFilePath.c_str());
    if (!xmlDoc.save_file(xmlFilePath.c_str()))
    {
        LOG_F(ERROR, "Failed to save XML file: %s", xmlFilePath.c_str());
        return false;
    }

    LOG_F(INFO, "JSON to XML conversion succeeded.");
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
        LOG_F(INFO, "JSON to XML conversion succeeded.");
    }
    else
    {
        LOG_F(INFO, "JSON to XML conversion failed.");
    }

    return 0;
}
