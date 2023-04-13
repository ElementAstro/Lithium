/*
 * configor.cpp
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

Date: 2023-4-4

Description: Configor

**************************************************/

#include <iostream>
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <sstream>
#include <spdlog/spdlog.h>
#include "nlohmann/json.hpp"

#include "configor.hpp"

using json = nlohmann::json;
namespace fs = std::filesystem;

namespace OpenAPT
{

    void ConfigManager::loadFromFile(const std::string &file_path)
    {
        std::ifstream ifs(file_path);
        if (!ifs.is_open())
        {
            spdlog::error("Failed to open file: {}", file_path);
            return;
        }
        json j;
        try
        {
            ifs >> j;
            const auto base_name = fs::path(file_path).stem().string();
            auto merged_j = json::object_t{};
            merged_j[base_name] = j;
            mergeConfig(merged_j);
        }
        catch (const json::exception &e)
        {
            spdlog::error("Failed to parse file: {}, error message: {}", file_path, e.what());
        }
    }

    void ConfigManager::loadFromDir(const std::string &dir_path, bool recursive)
    {
        for (const auto &file : fs::directory_iterator(dir_path))
        {
            const auto &path_str = file.path().string();
            if (file.is_directory() && recursive)
            {
                const auto &config_path = fs::path(path_str) / "config.json";
                if (fs::exists(config_path))
                {
                    loadFromFile(config_path.string());
                }
                else
                {
                    loadFromDir(path_str, true);
                }
            }
            else if (file.path().extension() == ".json")
            {
                loadFromFile(path_str);
            }
        }
    }

    void ConfigManager::setValue(const std::string &key_path, const json &value)
    {
        auto keys = split(key_path, "/");
        auto p = &config_;
        for (std::size_t i = 0; i < keys.size() - 1; ++i)
        {
            const auto &key = keys[i];
            if (!p->contains(key))
            {
                (*p)[key] = json::object();
            }
            p = &(*p)[key];
        }
        (*p)[keys.back()] = value;
    }

    json ConfigManager::getValue(const std::string &key_path) const
    {
        auto keys = split(key_path, "/");
        auto p = &config_;
        for (const auto &key : keys)
        {
            if (p->contains(key))
            {
                p = &(*p)[key];
            }
            else
            {
                spdlog::error("Key not found: {}", key_path);
                return nullptr;
            }
        }
        return *p;
    }

    void ConfigManager::deleteValue(const std::string &key_path)
    {
        auto keys = split(key_path, "/");
        auto p = &config_;
        for (std::size_t i = 0; i < keys.size() - 1; ++i)
        {
            const auto &key = keys[i];
            if (!p->contains(key))
            {
                spdlog::error("Key not found: {}", key_path);
                return;
            }
            p = &(*p)[key];
        }
        p->erase(keys.back());
    }

    void ConfigManager::printValue(const std::string &key, const json &value) const
    {
        if (value.is_object())
        {
            spdlog::debug("{}:", key);
            for (const auto &[sub_key, sub_value] : value.items())
            {
                const auto sub_key_path = key + "/" + sub_key;
                printValue(sub_key_path, sub_value);
            }
        }
        else
        {
            spdlog::debug("{}: {}", key, value.dump());
        }
    }

    std::vector<std::string> ConfigManager::split(const std::string &s, const std::string &delimiter) const
    {
        std::vector<std::string> tokens;
        std::size_t pos = 0;
        std::string temp_str = s; // 使用临时变量
        while ((pos = temp_str.find(delimiter)) != std::string::npos)
        {
            const auto token = temp_str.substr(0, pos);
            tokens.push_back(token);
            temp_str = temp_str.substr(pos + delimiter.length());
        }
        tokens.push_back(temp_str);
        return tokens;
    }

}
