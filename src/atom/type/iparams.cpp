/*
 * iparams.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-17

Description: Special Parameters Library for Atom

**************************************************/

#include "iparams.hpp"

#include <fstream>
#include <sstream>

#include <atom/type/json.hpp>
#include <atom/log/loguru.hpp>

using json = nlohmann::json;

bool IParams::serialize(const std::string &filename) const
{
    json jsonData;
    for (const auto &section : data)
    {
        json sectionData;
        for (const auto &entry : section.second)
        {
            if (entry.second.type() == typeid(int))
            {
                sectionData[entry.first] = std::any_cast<int>(entry.second);
            }
            else if (entry.second.type() == typeid(float))
            {
                sectionData[entry.first] = std::any_cast<float>(entry.second);
            }
            else if (entry.second.type() == typeid(double))
            {
                sectionData[entry.first] = std::any_cast<double>(entry.second);
            }
            else if (entry.second.type() == typeid(std::string))
            {
                sectionData[entry.first] = std::any_cast<std::string>(entry.second);
            }
            else if (entry.second.type() == typeid(const char *))
            {
                sectionData[entry.first] = std::any_cast<const char *>(entry.second);
            }
            else if (entry.second.type() == typeid(bool))
            {
                sectionData[entry.first] = std::any_cast<bool>(entry.second);
            }
            else
            {
                LOG_F(ERROR, "Unsupported type: {}", entry.second.type().name());
            }
        }
        jsonData[section.first] = sectionData;
    }

    std::ofstream file(filename);
    if (!file)
    {
        LOG_F(ERROR, "Failed to open file for writing: {}", filename);
        return false;
    }

    file << jsonData.dump(4);
    file.close();
    return true;
}

bool IParams::deserialize(const std::string &filename)
{
    std::ifstream file(filename);
    if (!file)
    {
        LOG_F(ERROR, "Failed to open file for reading: {}", filename);
        return false;
    }

    json jsonData;
    try
    {
        file >> jsonData;
    }
    catch (const json::parse_error &e)
    {
        LOG_F(ERROR, "Failed to parse JSON: {}", e.what());
        file.close();
        return false;
    }

    for (auto it = jsonData.begin(); it != jsonData.end(); ++it)
    {
        std::string section = it.key();
        for (auto entryIt = it.value().begin(); entryIt != it.value().end(); ++entryIt)
        {
            std::string key = entryIt.key();
            data[section][key] = entryIt.value();
        }
    }

    file.close();
    return true;
}

std::string IParams::toJson() const
{
    json jsonData;
    for (const auto &section : data)
    {
        json sectionData;
        for (const auto &entry : section.second)
        {
            try
            {
                if (entry.second.type() == typeid(int))
                {
                    sectionData[entry.first] = std::any_cast<int>(entry.second);
                }
                else if (entry.second.type() == typeid(float))
                {
                    sectionData[entry.first] = std::any_cast<float>(entry.second);
                }
                else if (entry.second.type() == typeid(double))
                {
                    sectionData[entry.first] = std::any_cast<double>(entry.second);
                }
                else if (entry.second.type() == typeid(std::string))
                {
                    sectionData[entry.first] = std::any_cast<std::string>(entry.second);
                }
                else if (entry.second.type() == typeid(const char *))
                {
                    sectionData[entry.first] = std::any_cast<const char *>(entry.second);
                }
                else if (entry.second.type() == typeid(bool))
                {
                    sectionData[entry.first] = std::any_cast<bool>(entry.second);
                }
                else
                {
                    throw std::runtime_error("Unsupported type");
                }
            }
            catch (const std::bad_any_cast &e)
            {
                LOG_F(ERROR, "Failed to cast any_cast: {}", e.what());
            }
        }
        jsonData[section.first] = sectionData;
    }

    return jsonData.dump(4);
}

bool IParams::fromJson(const std::string &jsonStr)
{
    json jsonData;
    try
    {
        jsonData = json::parse(jsonStr);
    }
    catch (const json::parse_error &e)
    {
        LOG_F(ERROR, "Failed to parse JSON: {}", e.what());
        return false;
    }

    for (auto it = jsonData.begin(); it != jsonData.end(); ++it)
    {
        std::string section = it.key();
        for (auto entryIt = it.value().begin(); entryIt != it.value().end(); ++entryIt)
        {
            std::string key = entryIt.key();
            data[section][key] = entryIt.value();
        }
    }

    return true;
}