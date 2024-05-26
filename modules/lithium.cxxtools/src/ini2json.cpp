/*
 * ini2json.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-29

Description: INI to JSON Converter

**************************************************/

#include "ini2json.hpp"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/string.hpp"

using namespace std;
using namespace std::chrono;
namespace fs = std::filesystem;

string tab(unsigned level) { return string(level * 4, ' '); }

bool iniToJson(const string& iniFilePath, const string& jsonFilePath) {
    LOG_F(INFO, "Converting INI file to JSON: {}", iniFilePath);
    if (!fs::exists(iniFilePath) || !fs::is_regular_file(iniFilePath)) {
        THROW_RUNTIME_ERROR("File not found: " + iniFilePath);
    }

    std::ifstream in(iniFilePath);
    std::ofstream out(jsonFilePath);
    if (!out.is_open()) {
        LOG_F(ERROR, "Can't create file: {}", jsonFilePath);
        return false;
    }

    out << "{" << endl;

    string line;
    bool sectionOpened = false;
    bool hasAttributes = false;

    while (getline(in, line)) {
        size_t commentPos = line.find(';');
        if (commentPos != string::npos)
            line = line.substr(0, commentPos);

        line = atom::utils::trim(line);

        if (line.empty())
            continue;

        if (line.front() == '[') {
            line = atom::utils::trim(line, "[]");

            if (hasAttributes) {
                hasAttributes = false;
                out << endl;
            }

            if (sectionOpened) {
                out << tab(1) << "}," << endl;
            } else {
                sectionOpened = true;
            }

            out << tab(1) << "\"" << line << "\": {" << endl;
        } else {
            auto pos = line.find('=');
            if (pos == string::npos)
                continue;

            string attribute = atom::utils::trim(line.substr(0, pos));
            string value = atom::utils::trim(line.substr(pos + 1));

            if (hasAttributes) {
                out << "," << endl;
            } else {
                hasAttributes = true;
            }

            out << tab(3) << "\"" << attribute << "\": ";
            if (value.find(':') != string::npos) {
                out << "{" << endl;
                auto items = atom::utils::explode(value, ',');
                for (const auto& item : items) {
                    auto kv = atom::utils::explode(item, ':');
                    if (kv.size() == 2)
                        out << tab(4) << "\"" << atom::utils::trim(kv[0])
                            << "\": \"" << atom::utils::trim(kv[1]) << "\","
                            << endl;
                }
                out.seekp(-2, out.cur);  // Remove the last comma
                out << endl << tab(3) << "}";
            } else if (value.find(',') != string::npos) {
                out << "[" << endl;
                auto items = atom::utils::explode(value, ',');
                for (const auto& item : items) {
                    out << tab(4) << "\"" << atom::utils::trim(item) << "\","
                        << endl;
                }
                out.seekp(-2, out.cur);  // Remove the last comma
                out << endl << tab(3) << "]";
            } else {
                out << "\"" << value << "\"";
            }
        }
    }

    if (hasAttributes) {
        out << endl;
    }

    if (sectionOpened) {
        out << tab(1) << "}" << endl;
    }

    out << "}" << endl;
    return true;
}

#if ATOM_STANDALONE_COMPONENT_ENABLED
#include <argparse/argparse.hpp>
int main(int argc, char** argv) {
    loguru::init(argc, argv);
    loguru::add_file("conversion_log.txt", loguru::Append, loguru::Verbosity_INFO);

    argparse::ArgumentParser program;
    program.add_argument("-i", "--input")
        .required()
        .help("path to input INI file");

    program.add_argument("-o", "--output")
        .required()
        .help("path to output JSON file");
    program.parse_args(argc, argv);

    std::string iniFilePath = program.get<std::string>("input");
    std::string jsonFilePath = program.get<std::string>("output");

    if (!iniToJson(iniFilePath, jsonFilePath)) {
        LOG_F(ERROR, "Conversion failed.");
        return 1;
    }

    LOG_F(INFO, "Conversion completed. Result has been saved to {}",
          jsonFilePath);

    return 0;
}
#else
bool ini_to_json(const std::string& ini_file, const std::string& json_file) {
    return iniToJson(ini_file, json_file);
}
#endif