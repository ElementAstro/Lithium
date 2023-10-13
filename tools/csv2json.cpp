#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include "nlohmann/json.hpp"
#include "argparse/argparse.hpp"
#include "loguru/loguru.hpp"

using json = nlohmann::json;

std::vector<std::string> splitString(const std::string &str, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string token;

    while (getline(ss, token, delimiter))
    {
        result.push_back(token);
    }

    return result;
}

json csvToJson(const std::string &csvFilePath)
{
    std::ifstream csvFile(csvFilePath);
    if (!csvFile.is_open())
    {
        throw std::runtime_error("Failed to open CSV file: " + csvFilePath);
    }

    std::vector<std::string> headers;
    std::vector<json> data;

    std::string line;
    bool isFirstLine = true;
    while (getline(csvFile, line))
    {
        std::vector<std::string> fields = splitString(line, ',');

        if (isFirstLine)
        {
            headers = fields;
            isFirstLine = false;
        }
        else
        {
            json row;
            for (size_t i = 0; i < fields.size(); ++i)
            {
                row[headers[i]] = fields[i];
            }
            data.push_back(row);
        }
    }

    json jsonData;
    for (const auto &row : data)
    {
        jsonData.push_back(row);
    }

    return jsonData;
}

void saveJsonToFile(const json &jsonData, const std::string &jsonFilePath)
{
    std::ofstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open())
    {
        throw std::runtime_error("Failed to open JSON file: " + jsonFilePath);
    }

    jsonFile << jsonData.dump(4);
    jsonFile.close();
}

int main(int argc, char *argv[])
{
    // 设置日志文件和级别
    std::string logFile = "conversion_log.txt";
    loguru::add_file(logFile.c_str(), loguru::Append, loguru::Verbosity_INFO);

    // 设置命令行参数解析器
    argparse::ArgumentParser program;
    program.add_argument("-i", "--input")
        .required()
        .help("path to input CSV file");

    program.add_argument("-o", "--output")
        .required()
        .help("path to output JSON file");
    program.parse_args(argc, argv);

    // 获取命令行参数
    std::string csvFilePath = program.get<std::string>("input");
    std::string jsonFilePath = program.get<std::string>("output");

    try
    {
        DLOG_F(INFO, "Converting CSV to JSON...");

        json jsonData = csvToJson(csvFilePath);
        saveJsonToFile(jsonData, jsonFilePath);

        DLOG_F(INFO, "CSV to JSON conversion succeeded.");
    }
    catch (const std::exception &ex)
    {
        DLOG_F(ERROR, "CSV to JSON conversion failed: %s", ex.what());
        return 1;
    }

    return 0;
}
