/*
 * json2toml.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-7-29

Description: JSON to TOML

**************************************************/

#include "json2toml.hpp"

#include <filesystem>
#include <fstream>

#include <toml++/toml.hpp>
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

void ConvertJsonToToml(const std::string &inputFile,
                       const std::string &outputFile) {
    try {
        const fs::path infile{inputFile};

        if (!fs::exists(infile)) {
            DLOG_F(ERROR, "Input file {} does not exist!", infile);
            return;
        }

        std::ifstream ifs{inputFile};
        json jsonData = json::parse(ifs);

        toml::value data = toml::from_json(jsonData);

        if (!outputFile.empty()) {
            std::ofstream out{outputFile};
            if (!out) {
                DLOG_F(ERROR, "Failed to open output file: {}", outputFile);
                return;
            }
            out << data << std::endl;
            DLOG_F(INFO, "Conversion completed. Output saved to {}",
                   outputFile);
        } else {
            std::cout << data << std::endl;
            DLOG_F(INFO, "Conversion completed. Result printed to stdout");
        }
    } catch (const std::exception &e) {
        DLOG_F(ERROR, "An exception occurred during conversion: {}", e.what());
    }
}

#if ATOM_STANDALONE_COMPONENT_ENABLED
#include <argparse/argparse.hpp>
int main(int argc, char **argv) {
    loguru::init(argc, argv);
    loguru::add_file("conversion_log.txt", loguru::Append, loguru::Verbosity_INFO);

    argparse::ArgumentParser program("json2toml");
    program.add_argument("inputFile").help("Input JSON file");
    program.add_argument("--outputFile", "-o")
        .nargs(1)
        .help("Output TOML file");

    try {
        program.parse_args(argc, argv);
    } catch (const std::runtime_error &err) {
        DLOG_F(ERROR, "{}", err.what());
        std::cout << program;
        return 1;
    }

    const std::string inputFile = program.get<std::string>("inputFile");
    const std::string outputFile = program.exist("outputFile")
                                       ? program.get<std::string>("outputFile")
                                       : "";

    ConvertJsonToToml(inputFile, outputFile);

    loguru::remove_all_callbacks();
    return 0;
}
#else
bool json_to_toml(const std::string &json_file, const std::string &toml_file) {
    if (json_file.empty() || toml_file.empty()) {
        LOG_F(ERROR, "json_file and toml_file must not be empty");
        return false;
    }
    if (!std::filesystem::exists(json_file) ||
        !std::filesystem::is_regular_file(json_file)) {
        LOG_F(ERROR, "json_file does not exist");
        return false;
    }
    if (std::filesystem::exists(toml_file)) {
        LOG_F(ERROR, "toml_file already exists");
        return false;
    }
    ConvertJsonToToml(json_file, toml_file);
    return true;
}
#endif
