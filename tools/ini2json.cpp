/*
 * ini2json.cpp
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

Description: INI to JSON

**************************************************/

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>

#include "loguru/loguru.hpp"

using namespace std::chrono;

std::string tab(unsigned level)
{
    return std::string(level * 4, ' ');
}

std::vector<std::string> explode(const std::string &text, const char symbol)
{
    std::vector<std::string> lines;
    std::string line;

    for (char c : text)
    {
        if (c == symbol)
        {
            lines.push_back(std::move(line));
            line.clear();
        }
        else
        {
            line.push_back(c);
        }
    }

    lines.push_back(std::move(line));
    return lines;
}

std::string trim(const std::string &line, const std::string &symbols = " \n\r\t")
{
    auto begin = line.find_first_not_of(symbols);
    auto end = line.find_last_not_of(symbols);

    if (begin == std::string::npos)
    {
        return "";
    }

    return line.substr(begin, end - begin + 1);
}

void usage(const std::string &binaryName)
{
    DLOG_F(ERROR, "Invalid number of arguments");
    DLOG_F(INFO, "Usage: %s <INI filename> [output filename]", binaryName.c_str());
}

int main(int argc, char **argv)
{
    loguru::init(argc, argv);

    if (argc < 2 || argc > 3)
    {
        DLOG_F(ERROR, "Invalid number of arguments");
        usage(argv[0]);
        return 1;
    }

    std::string inputFilename = argv[1];
    std::ifstream in(inputFilename, std::ios_base::binary | std::ios_base::in);

    if (!in.is_open())
    {
        DLOG_F(ERROR, "Can't open file: %s", inputFilename.c_str());
        return 1;
    }

    std::string outputFilename;

    if (argc == 3)
    {
        outputFilename = argv[2];
    }
    else
    {
        std::time_t now = system_clock::to_time_t(system_clock::now());
        std::stringstream ss;
        ss << std::put_time(std::localtime(&now), "%Y%m%d%H%M%S");
        outputFilename = "output_" + ss.str() + ".json";
    }

    DLOG_F(INFO, "Converting %s to %s", inputFilename.c_str(), outputFilename.c_str());

    std::ofstream out(outputFilename);
    if (!out.is_open())
    {
        DLOG_F(ERROR, "Can't create file: %s", outputFilename.c_str());
        return 1;
    }

    out << "{" << std::endl;

    std::string line;
    bool sectionOpened = false;
    bool hasAttributes = false;

    while (std::getline(in, line))
    {
        line = line.substr(0, line.find(';')); // remove comment
        line = trim(line);

        if (line.empty())
        {
            continue; // skip empty lines
        }

        if (line[0] == '[') // section name
        {
            line = trim(line, "[]");

            if (hasAttributes)
            {
                hasAttributes = false;
                out << std::endl;
            }

            if (sectionOpened)
            {
                out << tab(1) << "}," << std::endl;
            }
            else
            {
                sectionOpened = true;
            }

            out << tab(1) << "\"" << line << "\": {" << std::endl;
        }
        else // attribute line
        {
            if (line.find('=') == std::string::npos)
            {
                continue; // corrupted attributes line
            }

            std::string attribute = line.substr(0, line.find('='));
            std::string value = line.substr(line.find('=') + 1);

            if (hasAttributes)
            {
                out << "," << std::endl;
            }
            else
            {
                hasAttributes = true;
            }

            out << tab(3) << "\"" << attribute << "\": ";

            if (value.find(':') != std::string::npos) // associative array
            {
                out << "{" << std::endl;
                auto items = explode(value, ',');
                for (const auto &item : items)
                {
                    auto key = item.substr(0, item.find(':'));
                    auto value = item.substr(item.find(':') + 1);
                    out << tab(4) << "\"" << trim(key) << "\": \"" << trim(value) << "\"," << std::endl;
                }
                out << tab(3) << "}";
            }
            else if (value.find(',') != std::string::npos) // array
            {
                out << "[" << std::endl;
                auto items = explode(value, ',');
                for (const auto &item : items)
                {
                    out << tab(4) << "\"" << trim(item) << "\"," << std::endl;
                }
                out << tab(3) << "]";
            }
            else // simple value
            {
                out << "\"" << value << "\"";
            }
        }
    }

    if (hasAttributes)
    {
        out << std::endl;
    }

    if (sectionOpened)
    {
        out << tab(1) << "}" << std::endl;
    }

    out << "}" << std::endl;

    DLOG_F(INFO, "Conversion completed. Result has been saved to %s", outputFilename.c_str());

    return 0;
}
