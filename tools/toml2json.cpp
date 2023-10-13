/*
 * toml2json.cpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
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

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-7-29

Description: TOML to JSON

**************************************************/

#include <iostream>
#include <fstream>
#include <filesystem>

#include <toml++/toml.hpp>
#include <loguru/loguru.hpp>

namespace fs = std::filesystem;

static const char *usage = R"EOF(
    Usage:
        toml2json infile.toml [outfile.json]

    If outfile is not specificed then the output will be written to stdout
    )EOF";

void ConvertTomlToJson(const std::string &inputFile, const std::string &outputFile)
{
    try
    {
        const fs::path infile{inputFile};

        if (!fs::exists(infile))
        {
            DLOG_F(ERROR, "Input file %s does not exist!", infile.c_str());
            return;
        }

        auto data = toml::parse_file(infile.c_str());

        if (!outputFile.empty())
        {
            std::ofstream out{outputFile};
            if (!out)
            {
                DLOG_F(ERROR, "Failed to open output file: %s", outputFile.c_str());
                return;
            }
            out << toml::json_formatter(data) << std::endl;
            DLOG_F(INFO, "Conversion completed. Output saved to %s", outputFile.c_str());
        }
        else
        {
            std::cout << toml::json_formatter(data) << std::endl;
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
    loguru::add_file("toml2json.log", loguru::Append, loguru::Verbosity_INFO);

    if (argc < 2 || argc > 4)
    {
        DLOG_F(ERROR, "Error: incorrect number of arguments, got %d, expected 1 or 2.", argc - 1);
        std::cout << usage << std::endl;
        return 1;
    }

    const std::string inputFile{argv[1]};
    std::string outputFile;

    if (argc >= 3)
    {
        outputFile = argv[2];
    }

    ConvertTomlToJson(inputFile, outputFile);

    loguru::remove_all_callbacks();
    return 0;
}
