/*
 * iparams.hpp
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

Date: 2023-12-17

Description: Special Parameters Library for Atom

**************************************************/

#pragma once

#include <any>
#include <optional>
#include <string>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include <atom/type/json.hpp>

using json = nlohmann::json;

class IParams
{
public:
    /**
     * 设置参数
     * @tparam T 类型
     * @param section 部分名
     * @param key 键
     * @param value 值
     */
    template <typename T>
    void set(const std::string &section, const std::string &key, const T &value);

    /**
     * 获取参数的值
     * @tparam T 类型
     * @param section 部分名
     * @param key 键
     * @return 值，如果不存在则返回std::nullopt
     */
    template <typename T>
    std::optional<T> get(const std::string &section, const std::string &key) const;

    /**
     * 序列化参数到文件
     * @param filename 文件名
     * @return 是否成功
     */
    bool serialize(const std::string &filename) const;

    /**
     * 从文件反序列化参数
     * @param filename 文件名
     * @return 是否成功
     */
    bool deserialize(const std::string &filename);

    /**
     * 将参数序列化为 JSON 字符串
     * @return JSON 字符串
     */
    std::string toJson() const;

    /**
     * 从 JSON 字符串反序列化参数
     * @param jsonStr JSON 字符串
     * @return 是否成功
     */
    bool fromJson(const std::string &jsonStr);

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, emhash8::HashMap<std::string, std::any>> data; // 存储数据的映射表
#else
    std::unordered_map<std::string, std::unordered_map<std::string, std::any>> data; // 存储数据的映射表
#endif
};

template <typename T>
void IParams::set(const std::string &section, const std::string &key, const T &value)
{
    data[section][key] = value;
}

template <typename T>
std::optional<T> IParams::get(const std::string &section, const std::string &key) const
{
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
