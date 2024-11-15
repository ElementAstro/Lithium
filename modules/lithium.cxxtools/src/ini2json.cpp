// ini2json.cpp
#include "ini2json.hpp"

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/string.hpp"

#include <fstream>

namespace fs = std::filesystem;
using json = nlohmann::json;

namespace lithium::cxxtools::detail {

std::string tab(unsigned level) {
    return std::string(static_cast<size_t>(level * 4), ' ');
}

json Ini2Json::convertImpl(std::string_view iniFilePath) {
    LOG_F(INFO, "Converting INI file to JSON: {}", iniFilePath);
    if (!fs::exists(iniFilePath) || !fs::is_regular_file(iniFilePath)) {
        LOG_F(ERROR, "File not found or is not a regular file: {}",
              iniFilePath);
        THROW_FILE_NOT_FOUND("File not found or is not a regular file: {}",
                             iniFilePath);
    }

    std::ifstream in(iniFilePath.data());
    if (!in.is_open()) {
        LOG_F(ERROR, "Cannot open INI file: {}", iniFilePath);
        THROW_FILE_NOT_FOUND("Cannot open INI file: {}", iniFilePath);
    }

    json jsonData;
    std::string line;
    std::string currentSection;

    while (std::getline(in, line)) {
        auto commentPos = line.find(';');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        line = atom::utils::trim(line);

        if (line.empty()) {
            LOG_F(INFO, "Skipping empty line or comment.");
            continue;
        }

        if (line.front() == '[' && line.back() == ']') {
            currentSection = atom::utils::trim(line, "[]");
            jsonData[currentSection] = json::object();
            LOG_F(INFO, "Parsed section: [{}]", currentSection);
        } else {
            auto pos = line.find('=');
            if (pos == std::string::npos) {
                LOG_F(WARNING, "Skipping line without '=': {}", line);
                continue;
            }

            auto key = atom::utils::trim(line.substr(0, pos));
            auto value = atom::utils::trim(line.substr(pos + 1));
            jsonData[currentSection][key] = value;
            LOG_F(INFO, "Parsed key-value pair: {}={}", key, value);
        }
    }

    if (in.bad()) {
        LOG_F(ERROR, "Error occurred while reading INI file: {}", iniFilePath);
        THROW_RUNTIME_ERROR("Error occurred while reading INI file: {}",
                            iniFilePath);
    }

    LOG_F(INFO, "Successfully converted INI to JSON. Sections: {}",
          jsonData.size());
    return jsonData;
}

bool Ini2Json::saveToFileImpl(const json& jsonData,
                              std::string_view jsonFilePath) {
    LOG_F(INFO, "Saving JSON data to file: {}", jsonFilePath);
    std::ofstream out(jsonFilePath.data());
    if (!out.is_open()) {
        LOG_F(ERROR, "Can't create file: {}", jsonFilePath);
        THROW_FAIL_TO_OPEN_FILE("Can't create file: {}", jsonFilePath);
    }

    out << std::setw(4) << jsonData << std::endl;
    if (!out) {
        LOG_F(ERROR, "Failed to write JSON data to file: {}", jsonFilePath);
        THROW_FILE_NOT_WRITABLE("Failed to write JSON data to file: {}",
                                jsonFilePath);
    }

    LOG_F(INFO, "Successfully saved JSON data to file: {}", jsonFilePath);
    return true;
}

}  // namespace lithium::cxxtools::detail