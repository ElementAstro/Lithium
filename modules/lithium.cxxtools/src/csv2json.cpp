/*
 * csv2json.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-7

Description: CSV to JSON conversion

**************************************************/

#include "csv2json.hpp"

#include <fstream>
#include <sstream>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/string.hpp"

using json = nlohmann::json;

json csvToJson(const std::string &csvFilePath) {
    LOG_F(INFO, "Converting CSV file to JSON: {}", csvFilePath);
    std::ifstream csvFile(csvFilePath);
    if (!csvFile.is_open()) {
        THROW_RUNTIME_ERROR("Failed to open CSV file: " + csvFilePath);
    }

    std::vector<std::string_view> headers;
    std::vector<json> data;

    std::string line;
    bool isFirstLine = true;
    while (getline(csvFile, line)) {
        std::vector<std::string_view> fields =
            atom::utils::splitString(line, ',');

        if (isFirstLine) {
            headers = fields;
            isFirstLine = false;
        } else {
            json row;
            for (size_t i = 0; i < fields.size(); ++i) {
                row[headers[i]] = fields[i];
            }
            data.push_back(row);
        }
    }

    json jsonData;
    for (const auto &row : data) {
        jsonData.push_back(row);
    }
    LOG_F(INFO, "{}", jsonData.dump(4));

    return jsonData;
}

void saveJsonToFile(const json &jsonData, const std::string &jsonFilePath) {
    LOG_F(INFO, "Saving JSON data to file: {}", jsonFilePath);
    std::ofstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open()) {
        THROW_RUNTIME_ERROR("Failed to open JSON file: " + jsonFilePath);
    }
    jsonFile << jsonData.dump(4);
    jsonFile.close();
    LOG_F(INFO, "JSON data saved to file: {}", jsonFilePath);
}

#if ATOM_STANDALONE_COMPONENT_ENABLED
#include "argparse/argparse.hpp"
int main(int argc, char *argv[]) {
    loguru::init(argc, argv);
    loguru::add_file("conversion_log.txt", loguru::Append, loguru::Verbosity_INFO);

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

    try {
        DLOG_F(INFO, "Converting CSV to JSON...");

        json jsonData = csvToJson(csvFilePath);
        saveJsonToFile(jsonData, jsonFilePath);

        DLOG_F(INFO, "CSV to JSON conversion succeeded.");
    } catch (const std::exception &ex) {
        DLOG_F(ERROR, "CSV to JSON conversion failed: {}", ex.what());
        return 1;
    }

    return 0;
}
#else
bool csv_to_json(const std::string &csv_file, const std::string &json_file) {
    if (csv_file.empty() || json_file.empty()) {
        LOG_F(ERROR, "CSV to JSON conversion failed: invalid input file path");
        return false;
    }
    try {
        auto csv_data = csvToJson(csv_file);
        saveJsonToFile(csv_data, json_file);
        return true;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "CSV to JSON conversion failed: {}", e.what());
    }
    return false;
}
#endif