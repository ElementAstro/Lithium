/*
 * json2ini.cpp
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

Description: JSON to INI

**************************************************/

#include <fstream>
#include <unordered_map>
#include "atom/type/json.hpp"
#include "atom/log/loguru.hpp"

using json = nlohmann::json;

void writeIniSection(std::ofstream &iniFile, const std::string &sectionName, const json &jsonObject)
{
    iniFile << "[" << sectionName << "]" << std::endl;
    for (auto it = jsonObject.begin(); it != jsonObject.end(); ++it)
    {
        if (it->is_string())
        {
            iniFile << it.key() << "=" << it->get<std::string>() << std::endl;
        }
    }
    iniFile << std::endl;
}

void jsonToIni(const std::string &jsonFileName, const std::string &iniFileName)
{
    // 读取JSON文件
    std::ifstream jsonFile(jsonFileName);
    if (!jsonFile.is_open())
    {
        DLOG_F(ERROR, "Failed to open JSON file: %s", jsonFileName.c_str());
        return;
    }

    json jsonData;
    try
    {
        jsonFile >> jsonData;
    }
    catch (const std::exception &e)
    {
        DLOG_F(ERROR, "Failed to parse JSON file: %s. Error: %s", jsonFileName.c_str(), e.what());
        return;
    }

    // 将JSON转换为INI
    std::ofstream iniFile(iniFileName);
    if (!iniFile.is_open())
    {
        DLOG_F(ERROR, "Failed to create INI file: %s", iniFileName.c_str());
        return;
    }

    for (auto it = jsonData.begin(); it != jsonData.end(); ++it)
    {
        if (it->is_object())
        {
            writeIniSection(iniFile, it.key(), *it);
        }
    }

    // 关闭INI文件
    iniFile.close();
    if (!iniFile)
    {
        DLOG_F(ERROR, "Failed to save INI file: %s", iniFileName.c_str());
    }
    else
    {
        DLOG_F(INFO, "INI file is saved: %s", iniFileName.c_str());
    }
}

int main(int argc, char *argv[])
{
    // 初始化日志
    loguru::init(argc, argv);
    loguru::add_file("log.txt", loguru::Truncate, loguru::Verbosity_INFO);

    if (argc < 3)
    {
        DLOG_F(ERROR, "Usage: %s <json_file> <ini_file>", argv[0]);
        return 1;
    }

    const std::string jsonFileName = argv[1];
    const std::string iniFileName = argv[2];

    // 检查JSON文件是否存在
    std::ifstream inputFile(jsonFileName);
    if (!inputFile.is_open())
    {
        DLOG_F(ERROR, "JSON file not found: %s", jsonFileName.c_str());
        return 1;
    }
    inputFile.close();

    jsonToIni(jsonFileName, iniFileName);

    DLOG_F(INFO, "JSON to INI conversion is completed.");

    // 释放日志资源
    loguru::shutdown();

    return 0;
}
