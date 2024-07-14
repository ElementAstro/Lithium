/*
 * csv2json.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "csv2json.hpp"

#include <fstream>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/string.hpp"

namespace lithium::cxxtools::detail {
auto csvToJson(std::string_view csvFilePath) -> json {
    LOG_F(INFO, "Converting CSV file to JSON: {}", csvFilePath);
    std::ifstream csvFile(csvFilePath.data());
    if (!csvFile.is_open()) {
        THROW_RUNTIME_ERROR("Failed to open CSV file: ", csvFilePath);
    }

    std::vector<std::string> headers;
    std::vector<json> data;

    std::string line;
    bool isFirstLine = true;
    while (getline(csvFile, line)) {
        auto fields = atom::utils::splitString(line, ',');

        if (isFirstLine) {
            headers = fields;
            isFirstLine = false;
        } else {
            json row;
            for (size_t i = 0; i < fields.size(); ++i) {
                row[headers[i]] = fields[i];
            }
            data.push_back(std::move(row));
        }
    }

    return json{data};
}

void saveJsonToFile(const json &jsonData, std::string_view jsonFilePath) {
    LOG_F(INFO, "Saving JSON data to file: {}", jsonFilePath);
    std::ofstream jsonFile(jsonFilePath.data());
    if (!jsonFile.is_open() || !jsonFile.good()) {
        THROW_RUNTIME_ERROR("Failed to open JSON file: ", jsonFilePath);
    }
    jsonFile << jsonData.dump(4);
}
}  // namespace lithium::cxxtools::detail

#if ATOM_STANDALONE_COMPONENT_ENABLED
#include "argparse/argparse.hpp"
int main(int argc, char *argv[]) {
    loguru::init(argc, argv);
    loguru::add_file("conversion_log.txt", loguru::Append,
                     loguru::Verbosity_INFO);

    argparse::ArgumentParser program("csv2json");
    program.add_argument("-i", "--input")
        .required()
        .help("path to input CSV file");
    program.add_argument("-o", "--output")
        .required()
        .help("path to output JSON file");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        LOG_F(ERROR, "Error parsing arguments: {}", err.what());
        return 1;
    }

    std::string csvFilePath = program.get<std::string>("--input");
    std::string jsonFilePath = program.get<std::string>("--output");

    try {
        LOG_F(INFO, "Converting CSV to JSON...");
        auto jsonData = csvToJson(csvFilePath);
        saveJsonToFile(jsonData, jsonFilePath);
        LOG_F(INFO, "CSV to JSON conversion succeeded.");
    } catch (const std::exception &ex) {
        LOG_F(ERROR, "CSV to JSON conversion failed: {}", ex.what());
        return 1;
    }

    return 0;
}
#else
namespace lithium::cxxtools {
auto csvToJson(std::string_view csv_file, std::string_view json_file) -> bool {
    if (csv_file.empty() || json_file.empty()) {
        LOG_F(ERROR, "CSV to JSON conversion failed: invalid input file path");
        return false;
    }
    try {
        auto csvData = detail::csvToJson(csv_file);
        detail::saveJsonToFile(csvData, json_file);
        return true;
    } catch (const std::exception &e) {
        LOG_F(ERROR, "CSV to JSON conversion failed: {}", e.what());
    }
    return false;
}
}  // namespace lithium::cxxtools

#endif
