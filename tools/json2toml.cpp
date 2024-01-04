/*
 * json2toml.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Date: 2023-7-29

Description: JSON to TOML

**************************************************/

#include <iostream>
#include <fstream>
#include <filesystem>

#include <toml++/toml.hpp>
#include <loguru/loguru.hpp>
#include <argparse/argparse.hpp>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using json = nlohmann::json;

void ConvertJsonToToml(const std::string &inputFile, const std::string &outputFile)
{
    try
    {
        const fs::path infile{inputFile};

        if (!fs::exists(infile))
        {
            DLOG_F(ERROR, "Input file %s does not exist!", infile.c_str());
            return;
        }

        std::ifstream ifs{inputFile};
        json jsonData = json::parse(ifs);

        toml::value data = toml::from_json(jsonData);

        if (!outputFile.empty())
        {
            std::ofstream out{outputFile};
            if (!out)
            {
                DLOG_F(ERROR, "Failed to open output file: %s", outputFile.c_str());
                return;
            }
            out << data << std::endl;
            DLOG_F(INFO, "Conversion completed. Output saved to %s", outputFile.c_str());
        }
        else
        {
            std::cout << data << std::endl;
            DLOG_F(INFO, "Conversion completed. Result printed to stdout");
        }
    }
    catch (const std::exception &e)
    {
        DLOG_F(ERROR, "An exception occurred during conversion: %s", e.what());
    }
}

int main(int argc, char **argv)
{
    loguru::init(argc, argv);
    loguru::add_file("json2toml.log", loguru::Append, loguru::Verbosity_INFO);

    argparse::ArgumentParser program("json2toml");
    program.add_argument("inputFile")
        .help("Input JSON file");
    program.add_argument("--outputFile", "-o")
        .nargs(1)
        .help("Output TOML file");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        DLOG_F(ERROR, "%s", err.what());
        std::cout << program;
        return 1;
    }

    const std::string inputFile = program.get<std::string>("inputFile");
    const std::string outputFile = program.exist("outputFile") ? program.get<std::string>("outputFile") : "";

    ConvertJsonToToml(inputFile, outputFile);

    loguru::remove_all_callbacks();
    return 0;
}
