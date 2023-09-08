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

#pragma once

#include <fstream>
#include <mutex>
#include <shared_mutex>

#include "error/error_code.hpp"

#include "achievement_list.hpp"

#include "nlohmann/json.hpp"

namespace Lithium::Config
{
    class ConfigManager
    {
    public:
        ConfigManager();

        ~ConfigManager();

        static std::shared_ptr<ConfigManager> createShared();
        
        /**
         * @brief 从指定文件中加载JSON配置，并与原有配置进行合并
         *
         * Load JSON configuration from the specified file and merge with the existing configuration.
         *
         * @param path 配置文件路径
         */
        bool loadFromFile(const std::string &path);

        /**
         * @brief 加载指定目录下的所有JSON配置文件
         *
         * Load all JSON configuration files in the specified directory.
         *
         * @param dir_path 配置文件所在目录的路径
         */
        bool loadFromDir(const std::string &dir_path, bool recursive);

        /**
         * @brief 添加或更新一个配置项
         *
         * Add or update a configuration item.
         *
         * @param key_path 配置项的键路径，使用斜杠 / 进行分隔，如 "database/username"
         * @param value 配置项的值，使用 JSON 格式进行表示
         */
        void setValue(const std::string &key_path, const nlohmann::json &value);

        /**
         * @brief 获取一个配置项的值
         *
         * Get the value of a configuration item.
         *
         * @param key_path 配置项的键路径，使用斜杠 / 进行分隔，如 "database/username"
         * @return json 配置项对应的值，如果键不存在则返回 nullptr
         */
        nlohmann::json getValue(const std::string &key_path) const;

        /**
         * @brief 删除一个配置项
         *
         * Delete a configuration item.
         *
         * @param key_path 配置项的键路径，使用斜杠 / 进行分隔，如 "database/username"
         */
        void deleteValue(const std::string &key_path);

        /**
         * @brief 将当前配置保存到指定文件
         *
         * Save the current configuration to the specified file.
         *
         * @param file_path 目标文件路径
         */
        bool saveToFile(const std::string &file_path) const;

        /**
         * @brief 打印当前所有配置项
         *
         * Print all current configuration items.
         */
        void printAllValues() const;

        void tidyConfig();

    private:
        // JSON配置项
        nlohmann::json config_;
        std::mutex mutex_;
        mutable std::shared_timed_mutex rw_mutex_;

        std::shared_ptr<AAchievement::AchievementList> m_AchievementManager;

        void mergeConfig(const nlohmann::json &j);

        // 打印一个配置项的名称和值
        void printValue(const std::string &key, const nlohmann::json &value) const;

        /**
         * @brief 将字符串 s 按照指定的分隔符 delimiter 进行分割，并返回分割后的子字符串数组
         *
         * @param s 要被分割的字符串
         * @param delimiter 分隔符，用来指定在哪些位置对字符串进行分割
         * @return std::vector<std::string> 分割后的字符串数组，其中每个元素为分割后的一个子字符串
         *
         * Split the input string s into substrings based on the specified delimiter, and return an array of
         * substrings after splitting.
         *
         * @param s The string to be split.
         * @param delimiter The delimiter that specifies where to split the string.
         * @return std::vector<std::string> The array of substrings after splitting. Each element is a substring
         * obtained after splitting.
         */
        std::vector<std::string> split(const std::string &s, const std::string &delimiter) const;
    };
} // namespace Lithium
