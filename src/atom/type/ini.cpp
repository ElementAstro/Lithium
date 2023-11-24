/*
 * ini.cpp
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

Date: 2023-6-17

Description: INI File Read/Write Library

**************************************************/

#include "ini.hpp"

#include <fstream>
#include <sstream>

void INIFile::load(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + filename);
    }

    std::string line;
    std::string currentSection;
    while (std::getline(file, line))
    {
        parseLine(line, currentSection);
    }

    file.close();
}

void INIFile::save(const std::string &filename)
{
    std::ofstream file(filename);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to create file: " + filename);
    }
    
    for (const auto &section : data)
    {
        file << "[" << section.first << "]\n";
        for (const auto &entry : section.second)
        {
            if (entry.second.type() == typeid(int))
            {
                file << entry.first << "=" << std::any_cast<int>(entry.second) << "\n";
            }
            else if (entry.second.type() == typeid(float))
            {
                file << entry.first << "=" << std::any_cast<float>(entry.second) << "\n";
            }
            else if (entry.second.type() == typeid(double))
            {
                file << entry.first << "=" << std::any_cast<double>(entry.second) << "\n";
            }
            else if (entry.second.type() == typeid(std::string))
            {
                file << entry.first << "=" << std::any_cast<std::string>(entry.second) << "\n";
            }
            else if (entry.second.type() == typeid(const char*))
            {
                file << entry.first << "=" << std::any_cast<const char*>(entry.second) << "\n";
            }
            else if (entry.second.type() == typeid(bool))
            {
                file << entry.first << "=" << std::any_cast<bool>(entry.second) << "\n";
            }
            else
            {
                throw std::runtime_error("Unsupported type");
            }
        }
        file << "\n";
    }

    file.close();
}

void INIFile::parseLine(const std::string &line, std::string &currentSection)
{
    std::stringstream ss(line);
    std::string token;
    if (std::getline(ss, token, '='))
    {
        // 去除字符串前后的空格和制表符
        token = trim(token);
        if (token[0] == '[' && token[token.length() - 1] == ']')
        {
            currentSection = token.substr(1, token.length() - 2);
        }
        else
        {
            std::string key = trim(token);
            std::string value;
            if (std::getline(ss, value))
            {
                value = trim(value);
                data[currentSection][key] = value;
            }
        }
    }
}

std::string INIFile::trim(const std::string &str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");
    if (start == std::string::npos || end == std::string::npos)
    {
        return "";
    }
    else
    {
        return str.substr(start, end - start + 1);
    }
}

/*
int main()
{
    INIFile ini;
    try
    {
        ini.load("config.ini");

        // 获取配置项
        std::optional<std::string> usernameOpt = ini.get<std::string>("User", "Username");
        std::optional<std::string> passwordOpt = ini.get<std::string>("User", "Password");

        if (usernameOpt.has_value() && passwordOpt.has_value())
        {
            std::cout << "Username: " << usernameOpt.value() << std::endl;
            std::cout << "Password: " << passwordOpt.value() << std::endl;
        }
        else
        {
            std::cout << "Username or Password not found" << std::endl;
        }

        // 修改配置项
        ini.set("User", "Password", std::string("new_pas"));

        // 存储到文件
        ini.save("config_modified.ini");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

*/