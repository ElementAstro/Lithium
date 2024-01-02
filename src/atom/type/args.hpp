/*
 * args.hpp
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

Date: 2023-12-28

Description: Argument Container Library for C++

**************************************************/

#pragma once

#include <any>
#include <optional>
#include <string>
#include <vector>
#ifdef ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

// 设置参数的便捷宏
#define SET_ARGUMENT(container, name, value) container.set(#name, value)

// 获取参数的便捷宏
#define GET_ARGUMENT(container, name, type) container.get<type>(#name).value_or(type{})

// 检查参数是否存在的便捷宏
#define HAS_ARGUMENT(container, name) container.contains(#name)

// 删除参数的便捷宏
#define REMOVE_ARGUMENT(container, name) container.remove(#name)

class ArgumentContainer
{
public:
    /**
     * @brief 设置参数值。
     * @tparam T 参数的类型。
     * @param name 参数的名称。
     * @param value 参数的值。
     */
    template <typename T>
    void set(const std::string &name, const T &value)
    {
        m_arguments[name] = value;
    }

    /**
     * @brief 获取参数值。
     * @tparam T 参数的类型。
     * @param name 参数的名称。
     * @return 参数的值（如果存在），否则返回std::nullopt。
     */
    template <typename T>
    std::optional<T> get(const std::string &name) const
    {
        auto it = m_arguments.find(name);
        if (it != m_arguments.end())
        {
            try
            {
                return std::any_cast<T>(it->second);
            }
            catch (const std::bad_any_cast &)
            {
                return std::nullopt;
            }
        }
        else
        {
            return std::nullopt;
        }
    }

    /**
     * @brief 移除指定的参数。
     * @param name 要移除的参数的名称。
     * @return 如果成功移除参数，则返回true；否则返回false。
     */
    bool remove(const std::string &name)
    {
        return m_arguments.erase(name) != 0;
    }

    /**
     * @brief 检查参数是否存在。
     * @param name 要检查的参数的名称。
     * @return 如果参数存在，则返回true；否则返回false。
     */
    bool contains(const std::string &name) const
    {
        return m_arguments.count(name) != 0;
    }

    /**
     * @brief 获取参数的数量。
     * @return 参数的数量。
     */
    std::size_t size() const
    {
        return m_arguments.size();
    }

    /**
     * @brief 获取所有参数的名称。
     * @return 包含所有参数名称的字符串向量。
     */
    std::vector<std::string> getNames() const
    {
        std::vector<std::string> names;
        names.reserve(m_arguments.size());
        for (const auto &pair : m_arguments)
        {
            names.push_back(pair.first);
        }
        return names;
    }

    /**
     * @brief 重载索引运算符[]以获取和设置参数值。
     * @tparam T 参数的类型。
     * @param name 参数的名称。
     * @return 参数的引用。
     */
    template <typename T>
    T &operator[](const std::string &name)
    {
        return std::any_cast<T &>(m_arguments[name]);
    }

    /**
     * @brief 重载赋值运算符=以设置参数值。
     * @tparam T 参数的类型。
     * @param argument 要设置的参数（名称和值）。
     */
    template <typename T>
    void operator=(const std::pair<std::string, T> &argument)
    {
        set(argument.first, argument.second);
    }

    void operator=(const std::unordered_map<std::string, std::any> &container)
    {
        m_arguments = container;
    }

private:
#ifdef ENABLE_FASTHASH
    emhash::HashMap<std::string, std::any> m_arguments;
#else
    std::unordered_map<std::string, std::any> m_arguments;
#endif
};

using Args = ArgumentContainer;
