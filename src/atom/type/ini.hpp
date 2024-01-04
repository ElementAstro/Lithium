/*
 * ini.hpp
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

Date: 2023-6-17

Description: INI File Read/Write Library

**************************************************/

#pragma once

#include <any>
#include <optional>
#include <string>
#include <mutex>
#include <shared_mutex>
#ifdef ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

class INIFile
{
public:
    /**
     * @brief 加载INI文件
     * @brief load INI file
     * @param filename 文件名
     * @param filename the name of file
     */
    void load(const std::string &filename);

    /**
     * @brief 保存INI文件
     * @brief save INI file
     * @param filename 文件名
     * @param filename the name of file
     */
    void save(const std::string &filename);

    /**
     * @brief 设置INI文件中的值
     * @tparam T 类型
     * @param section 部分名
     * @param key 键
     * @param value 值
     */
    template <typename T>
    void set(const std::string &section, const std::string &key, const T &value);

    /**
     * @brief 获取INI文件中的值
     * @tparam T 类型
     * @param section 部分名
     * @param key 键
     * @return 值，如果不存在则返回std::nullopt
     */
    template <typename T>
    std::optional<T> get(const std::string &section, const std::string &key) const;

    /**
     * @brief 判断INI文件中是否存在指定键
     * @param section 部分名
     * @param key 键
     * @return 存在返回true，否则返回false
     */
    bool has(const std::string &section, const std::string &key) const;

    /**
     * 判断INI文件中是否存在指定部分
     * @param section 部分名
     * @return 存在返回true，否则返回false
     */
    bool hasSection(const std::string &section) const;

    /**
     * @brief 获取INI文件中的部分
     * @param section 部分名
     * @return 部分内容
     */
    std::unordered_map<std::string, std::any> operator[](const std::string &section)
    {
        return data[section];
    }

    /**
     * @brief 将INI文件中的数据转换为JSON字符串
     * @return JSON字符串
     */
    std::string toJson() const;

    /**
     * @brief 将INI文件中的数据转换为XML字符串
     * @return XML字符串
     */
    std::string toXml() const;

private:
#ifdef ENABlE_FASTHASH
    emhash8::HashMap<std::string, emhash8::HashMap<std::string, std::any>> data; // 存储数据的映射表
#else
    std::unordered_map<std::string, std::unordered_map<std::string, std::any>> data; // 存储数据的映射表
#endif

    mutable std::shared_mutex m_sharedMutex; // 共享互斥锁，用于线程安全
    /**
     * @brief 解析INI文件的一行，并更新当前部分
     * @param line 行内容
     * @param currentSection 当前部分
     */
    void parseLine(const std::string &line, std::string &currentSection);
    /**
     * @brief 剔除字符串前后的空格
     * @param str 字符串
     * @return 剔除空格后的字符串
     */
    std::string trim(const std::string &str);
};

template <typename T>
void INIFile::set(const std::string &section, const std::string &key, const T &value)
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    data[section][key] = value;
}

template <typename T>
std::optional<T> INIFile::get(const std::string &section, const std::string &key) const
{
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    auto it = data.find(section);
    if (it != data.end())
    {
        auto entryIt = it->second.find(key);
        if (entryIt != it->second.end())
        {
            try
            {
                return std::any_cast<T>(entryIt->second);
            }
            catch (const std::bad_any_cast &)
            {
                return std::nullopt;
            }
        }
    }
    return std::nullopt;
}