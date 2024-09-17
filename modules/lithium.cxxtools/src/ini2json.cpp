/*
 * ini2json.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

#include "ini2json.hpp"

#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <ranges>
#include <string>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/utils/string.hpp"

namespace fs = std::filesystem;

namespace lithium::cxxtools::detail {

auto tab(unsigned level) -> std::string {
    return std::string(static_cast<size_t>(level * 4), ' ');
}

auto iniToJson(std::string_view iniFilePath, std::string_view jsonFilePath,
               char commentChar = ';') -> bool {
    LOG_F(INFO, "Converting INI file to JSON: {}", iniFilePath);
    if (!fs::exists(iniFilePath) || !fs::is_regular_file(iniFilePath)) {
        THROW_FILE_NOT_FOUND("File not found: ", iniFilePath);
    }

    std::ifstream in(iniFilePath.data());
    std::ofstream out(jsonFilePath.data());
    if (!out.is_open()) {
        LOG_F(ERROR, "Can't create file: {}", jsonFilePath);
        return false;
    }

    out << "{" << std::endl;

    std::string line;
    bool sectionOpened = false;
    bool hasAttributes = false;

    while (std::getline(in, line)) {
        auto commentPos = line.find(commentChar);
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        line = atom::utils::trim(line);

        if (line.empty()) {
            continue;
        }

        if (line.front() == '[') {
            line = atom::utils::trim(line, "[]");

            if (hasAttributes) {
                hasAttributes = false;
                out << std::endl;
            }

            if (sectionOpened) {
                out << tab(1) << "}," << std::endl;
            } else {
                sectionOpened = true;
            }

            out << tab(1) << std::format("\"{}\": {{", line) << std::endl;
        } else {
            auto pos = line.find('=');
            if (pos == std::string::npos) {
                continue;
            }

            auto attribute = atom::utils::trim(line.substr(0, pos));
            auto value = atom::utils::trim(line.substr(pos + 1));

            if (hasAttributes) {
                out << "," << std::endl;
            } else {
                hasAttributes = true;
            }

            out << tab(2) << std::format("\"{}\": ", attribute);
            if (value.find(':') != std::string::npos) {
                out << "{" << std::endl;
                for (const auto& item : atom::utils::explode(value, ',')) {
                    auto kv = atom::utils::explode(item, ':');
                    if (kv.size() == 2) {
                        out << tab(3)
                            << std::format(R"("{}": "{}",)",
                                           atom::utils::trim(kv[0]),
                                           atom::utils::trim(kv[1]))
                            << std::endl;
                    }
                }
                out.seekp(-2, std::ofstream::cur);  // Remove the last comma
                out << std::endl << tab(2) << "}";
            } else if (value.find(',') != std::string::npos) {
                out << "[" << std::endl;
                for (const auto& item : atom::utils::explode(value, ',')) {
                    out << tab(3)
                        << std::format("\"{}\",", atom::utils::trim(item))
                        << std::endl;
                }
                out.seekp(-2, std::ofstream::cur);  // Remove the last comma
                out << std::endl << tab(2) << "]";
            } else {
                out << std::format("\"{}\"", value);
            }
        }
    }

    if (hasAttributes) {
        out << std::endl;
    }

    if (sectionOpened) {
        out << tab(1) << "}" << std::endl;
    }

    out << "}" << std::endl;
    return true;
}
}  // namespace lithium::cxxtools::detail

#if ATOM_STANDALONE_COMPONENT_ENABLED
#include <argparse/argparse.hpp>
int main(int argc, char** argv) {
    loguru::init(argc, argv);
    loguru::add_file("conversion_log.txt", loguru::Append,
                     loguru::Verbosity_INFO);

    argparse::ArgumentParser program("ini2json");
    program.add_argument("-i", "--input")
        .required()
        .help("path to input INI file");
    program.add_argument("-o", "--output")
        .required()
        .help("path to output JSON file");
    program.add_argument("-c", "--comment")
        .default_value(';')
        .help("comment character used in the INI file");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error& err) {
        LOG_F(ERROR, "Error parsing arguments: {}", err.what());
        return 1;
    }

    std::string iniFilePath = program.get<std::string>("--input");
    std::string jsonFilePath = program.get<std::string>("--output");
    char commentChar = program.get<char>("--comment");

    try {
        LOG_F(INFO, "Converting INI to JSON...");
        if (!lithium::cxxtools::detail::iniToJson(iniFilePath, jsonFilePath,
                                                  commentChar)) {
            LOG_F(ERROR, "Conversion failed.");
            return 1;
        }
        LOG_F(INFO, "Conversion completed. Result has been saved to {}",
              jsonFilePath);
    } catch (const std::exception& ex) {
        LOG_F(ERROR, "Conversion failed: {}", ex.what());
        return 1;
    }

    return 0;
}
#else
namespace lithium::cxxtools {
auto iniToJson(std::string_view ini_file, std::string_view json_file,
               char commentChar = ';') -> bool {
    return detail::iniToJson(ini_file, json_file, commentChar);
}
}  // namespace lithium::cxxtools

#endif