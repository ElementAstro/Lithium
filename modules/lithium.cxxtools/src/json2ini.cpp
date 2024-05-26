/*
 * json2ini.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-29

Description: JSON to INI

**************************************************/

#include "json2ini.hpp"

#include <fstream>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;

void writeIniSection(std::ofstream &iniFile, const std::string &sectionName,
                     const json &jsonObject) {
    iniFile << "[" << sectionName << "]" << std::endl;
    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
        if (it->is_string()) {
            iniFile << it.key() << "=" << it->get<std::string>() << std::endl;
        }
    }
    iniFile << std::endl;
}

void jsonToIni(const std::string &jsonFilePath,
               const std::string &iniFilePath) {
    std::ifstream jsonFile(jsonFilePath);
    if (!jsonFile.is_open()) {
        LOG_F(ERROR, "Failed to open JSON file: {}", jsonFilePath);
        return;
    }

    json jsonData;
    try {
        jsonFile >> jsonData;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "Failed to parse JSON file: {}. Error: {}", jsonFilePath,
              e.what());
        return;
    }

    std::ofstream iniFile(iniFilePath);
    if (!iniFile.is_open()) {
        LOG_F(ERROR, "Failed to create INI file: {}", iniFilePath);
        return;
    }

    for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
        if (it->is_object()) {
            writeIniSection(iniFile, it.key(), *it);
        }
    }

    iniFile.close();
    if (!iniFile) {
        LOG_F(ERROR, "Failed to save INI file: {}", iniFilePath);
    } else {
        LOG_F(INFO, "INI file is saved: {}", iniFilePath);
    }
}

#if ATOM_STANDALONE_COMPONENT_ENABLED
#include <argparse/argparse.hpp>
int main(int argc, char *argv[]) {
    loguru::init(argc, argv);
    loguru::add_file("conversion_log.txt", loguru::Append, loguru::Verbosity_INFO);

    argparse::ArgumentParser program;
    program.add_argument("-i", "--input")
        .required()
        .help("path to input CSV file");

    program.add_argument("-o", "--output")
        .required()
        .help("path to output JSON file");
    program.parse_args(argc, argv);

    std::string jsonFilePath = program.get<std::string>("input");
    std::string iniFilePath = program.get<std::string>("output");

    std::ifstream inputFile(jsonFilePath);
    if (!inputFile.is_open()) {
        LOG_F(ERROR, "JSON file not found: {}", jsonFilePath);
        return 1;
    }
    inputFile.close();
    jsonToIni(jsonFilePath, iniFilePath);

    LOG_F(INFO, "JSON to INI conversion is completed.");
    return 0;
}
#else
bool json_to_ini(const std::string &json_file, const std::string &ini_file) {
    try {
        LOG_F(INFO, "Converting JSON to INI...");
        jsonToIni(json_file, ini_file);
        LOG_F(INFO, "JSON to INI conversion is completed.");
        return true;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "JSON to INI conversion failed: {}", e.what());
    }
    return false;
}
#endif
