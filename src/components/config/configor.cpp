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

Date: 2023-4-30

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

    void ConfigManager::loadFromFile(const std::string &path)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open())
        {
            spdlog::error("Failed to open file: {}", path);
            return;
        }
        json j;
        try
        {
            ifs >> j;
            // 获取文件名并去掉后缀
            const std::string basename = path.substr(path.find_last_of("/\\") + 1);
            const std::string name_without_ext = basename.substr(0, basename.find_last_of('.'));
            auto merged_j = json::object_t{};
            merged_j[name_without_ext] = j;
            mergeConfig(merged_j);
        }
        catch (const json::exception &e)
        {
            spdlog::error("Failed to parse file: {}, error message: {}", path, e.what());
        }
    }

    void ConfigManager::loadFromDir(const std::string &dir_path, bool recursive)
    {
        for (const auto &file : fs::directory_iterator(dir_path))
        {
            if (file.path().extension() == ".json")
            {
                loadFromFile(file.path().string());
            }
            else if (recursive && file.is_directory())
            {
                const std::string subdir_path = file.path().string();
                const std::string basename = file.path().filename().string();
                const std::string config_file_path = subdir_path + "/config.json";
                if (fs::exists(config_file_path))
                {
                    json j;
                    try
                    {
                        std::ifstream ifs(config_file_path);
                        ifs >> j;
                        auto merged_j = json::object_t{};
                        merged_j[dir_path][basename] = j;
                        mergeConfig(merged_j);
                    }
                    catch (const json::exception &e)
                    {
                        spdlog::error("Failed to parse file: {}, error message: {}", config_file_path, e.what());
                    }
                }
                loadFromDir(subdir_path, true);
            }
        }
    }

    void ConfigManager::setValue(const std::string &key_path, const json &value)
    {
        std::vector<std::string> keys = split(key_path, "/");
        json *p = &config_;
        for (int i = 0; i < keys.size() - 1; ++i)
        {
            if (p->contains(keys[i]))
            {
                p = &(*p)[keys[i]];
            }
            else
            {
                (*p)[keys[i]] = json();
                p = &(*p)[keys[i]];
            }
        }
        (*p)[keys.back()] = value;
    }

    json ConfigManager::getValue(const std::string &key_path) const
    {
        std::vector<std::string> keys = split(key_path, "/");
        const json *p = &config_;
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
        std::vector<std::string> keys = split(key_path, "/");
        json *p = &config_;
        for (int i = 0; i < keys.size() - 1; ++i)
        {
            if (p->contains(keys[i]))
            {
                p = &(*p)[keys[i]];
            }
            else
            {
                spdlog::error("Key not found: {}", key_path);
                return;
            }
        }
        p->erase(keys.back());
    }

    void ConfigManager::printValue(const std::string &key, const json &value) const
    {
        if (value.is_object())
        {
            spdlog::debug("{}:", key);
            for (auto &[sub_key, sub_value] : value.items())
            {
                std::stringstream ss;
                ss << key << "/" << sub_key;
                printValue(ss.str(), sub_value);
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
        std::string tempStr = s; // 新增代码，使用临时变量存储字符串 s 的值
        while ((pos = tempStr.find(delimiter)) != std::string::npos)
        {
            tokens.push_back(tempStr.substr(0, pos));
            tempStr = tempStr.substr(pos + delimiter.length());
        }
        tokens.push_back(tempStr);
        return tokens;
    }
}
