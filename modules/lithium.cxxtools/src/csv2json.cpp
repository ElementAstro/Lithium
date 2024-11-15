// csv2json.cpp
#include "csv2json.hpp"

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/string.hpp"

#include <fstream>

using json = nlohmann::json;

namespace lithium::cxxtools::detail {

json Csv2Json::convertImpl(std::string_view csvFilePath) {
    LOG_F(INFO, "Converting CSV file to JSON: {}", csvFilePath);
    std::ifstream csvFile(csvFilePath.data());
    if (!csvFile.is_open()) {
        LOG_F(ERROR, "Failed to open CSV file: {}", csvFilePath);
        THROW_RUNTIME_ERROR("Failed to open CSV file: {}", csvFilePath);
    }

    std::vector<std::string> headers;
    json data = json::array();
    std::string line;
    bool isFirstLine = true;

    while (std::getline(csvFile, line)) {
        if (line.empty()) {
            LOG_F(WARNING, "Skipping empty line in CSV file.");
            continue;
        }

        auto fields = atom::utils::splitString(line, ',');

        if (isFirstLine) {
            headers = fields;
            isFirstLine = false;
            LOG_F(INFO, "Parsed CSV headers: {}", headers);
        } else {
            if (fields.size() != headers.size()) {
                LOG_F(WARNING,
                      "Mismatch between number of fields and headers. Line: {}",
                      line);
                continue;
            }

            json row;
            for (size_t i = 0; i < fields.size(); ++i) {
                row[headers[i]] = fields[i];
            }
            data.push_back(std::move(row));
            LOG_F(INFO, "Parsed CSV row: {}", row.dump());
        }
    }

    if (csvFile.bad()) {
        LOG_F(ERROR, "Error occurred while reading CSV file: {}", csvFilePath);
        THROW_RUNTIME_ERROR("Error occurred while reading CSV file: {}",
                            csvFilePath);
    }

    LOG_F(INFO, "Successfully converted CSV to JSON. Total rows: {}",
          data.size());
    return data;
}

bool Csv2Json::saveToFileImpl(const json& jsonData,
                              std::string_view jsonFilePath) {
    LOG_F(INFO, "Saving JSON data to file: {}", jsonFilePath);
    std::ofstream jsonFile(jsonFilePath.data());
    if (!jsonFile.is_open() || !jsonFile.good()) {
        LOG_F(ERROR, "Failed to open JSON file for writing: {}", jsonFilePath);
        THROW_FAIL_TO_OPEN_FILE("Failed to open JSON file for writing: {}",
                                jsonFilePath);
    }

    jsonFile << std::setw(4) << jsonData << std::endl;
    if (!jsonFile) {
        LOG_F(ERROR, "Failed to write JSON data to file: {}", jsonFilePath);
        THROW_FILE_NOT_WRITABLE("Failed to write JSON data to file: {}",
                                jsonFilePath);
    }

    LOG_F(INFO, "Successfully saved JSON data to file: {}", jsonFilePath);
    return true;
}

}  // namespace lithium::cxxtools::detail