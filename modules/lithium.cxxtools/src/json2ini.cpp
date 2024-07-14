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

#include <filesystem>
#include <fstream>

#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "exception.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace lithium::cxxtools::detail {
void writeIniSection(std::ofstream &iniFile, std::string_view sectionName,
                     const json &jsonObject) {
    iniFile << "[" << sectionName << "]" << std::endl;
    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it) {
        if (it->is_string()) {
            iniFile << it.key() << "=" << it->get<std::string>() << std::endl;
        }
    }
    iniFile << std::endl;
}

void jsonToIni(std::string_view jsonFilePath, std::string_view iniFilePath) {
    if (!fs::exists(jsonFilePath) || !fs::is_regular_file(jsonFilePath)) {
        THROW_FILE_NOT_FOUND("JSON file not found: ", jsonFilePath);
    }

    std::ifstream jsonFile(jsonFilePath.data());
    if (!jsonFile.is_open()) {
        THROW_FILE_NOT_READABLE("Failed to open JSON file: ", jsonFilePath);
    }

    json jsonData;
    try {
        jsonFile >> jsonData;
    } catch (const std::exception &e) {
        THROW_RUNTIME_ERROR("Failed to parse JSON file: ", jsonFilePath,
                            ". Error: ", e.what());
    }

    std::ofstream iniFile(iniFilePath.data());
    if (!iniFile.is_open()) {
        THROW_RUNTIME_ERROR("Failed to create INI file: ", iniFilePath);
    }

    for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
        if (it->is_object()) {
            writeIniSection(iniFile, it.key(), *it);
        }
    }

    if (!iniFile) {
        THROW_FILE_NOT_WRITABLE("Failed to save INI file: ", iniFilePath);
    }
    LOG_F(INFO, "INI file is saved: {}", iniFilePath);
}
}  // namespace lithium::cxxtools::detail

#if ATOM_STANDALONE_COMPONENT_ENABLED
#include <argparse/argparse.hpp>
int main(int argc, char *argv[]) {
    loguru::init(argc, argv);
    loguru::add_file("conversion_log.txt", loguru::Append,
                     loguru::Verbosity_INFO);

    argparse::ArgumentParser program("json2ini");
    program.add_argument("-i", "--input")
        .required()
        .help("path to input JSON file");
    program.add_argument("-o", "--output")
        .required()
        .help("path to output INI file");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        LOG_F(ERROR, "Error parsing arguments: {}", err.what());
        return 1;
    }

    std::string jsonFilePath = program.get<std::string>("--input");
    std::string iniFilePath = program.get<std::string>("--output");

    try {
        LOG_F(INFO, "Converting JSON to INI...");
        jsonToIni(jsonFilePath, iniFilePath);
        LOG_F(INFO, "JSON to INI conversion completed.");
    } catch (const std::exception &ex) {
        LOG_F(ERROR, "JSON to INI conversion failed: {}", ex.what());
        return 1;
    }

    return 0;
}
#else
namespace lithium::cxxtools {
auto jsonToIni(std::string_view jsonFilePath,
               std::string_view iniFilePath) -> bool {
    try {
        LOG_F(INFO, "Converting JSON to INI...");
        detail::jsonToIni(jsonFilePath, iniFilePath);
        LOG_F(INFO, "JSON to INI conversion completed.");
        return true;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "JSON to INI conversion failed: {}", e.what());
    }
    return false;
}
}  // namespace lithium::cxxtools

#endif
