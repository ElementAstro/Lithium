/*
 * toml2json.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-29

Description: TOML to JSON

**************************************************/

#include <filesystem>
#include <fstream>
#include <iostream>

#include <toml++/toml.hpp>
#include "atom/log/loguru.hpp"

namespace fs = std::filesystem;

void ConvertTomlToJson(const std::string &inputFile,
                       const std::string &outputFile) {
    try {
        const fs::path infile{inputFile};

        if (!fs::exists(infile)) {
            DLOG_F(ERROR, "Input file {} does not exist!", infile);
            return;
        }

        auto data = toml::parse_file(infile);

        if (!outputFile.empty()) {
            std::ofstream out{outputFile};
            if (!out) {
                DLOG_F(ERROR, "Failed to open output file: {}", outputFile);
                return;
            }
            out << toml::json_formatter(data) << std::endl;
            DLOG_F(INFO, "Conversion completed. Output saved to {}",
                   outputFile);
        } else {
            std::cout << toml::json_formatter(data) << std::endl;
            DLOG_F(INFO, "Conversion completed. Result printed to stdout");
        }
    } catch (const std::exception &e) {
        DLOG_F(ERROR, "An exception occurred during conversion: {}", e.what());
    }
}

#if ATOM_STANDALONE_COMPONENT_ENABLED
#include "argparse/argparse.hpp"
int main(int argc, char *argv[]) {
    loguru::init(argc, argv);
    loguru::add_file("conversion_log.txt", loguru::Append,
                     loguru::Verbosity_INFO);

    argparse::ArgumentParser program;
    program.add_argument("-i", "--input")
        .required()
        .help("path to input TOML file");

    program.add_argument("-o", "--output")
        .required()
        .help("path to output JSON file");
    program.parse_args(argc, argv);

    std::string tomlFilePath = program.get<std::string>("input");
    std::string jsonFilePath = program.get<std::string>("output");

    ConvertTomlToJson(tomlFilePath, jsonFilePath);

    return 0;
}
#else
bool toml_to_json(const std::string &toml_file, const std::string &json_file) {
    ConvertTomlToJson(toml_file, json_file);
    return true;
}

#endif
