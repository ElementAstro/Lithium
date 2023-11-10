/*
 * variables.hpp
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

Date: 2023-11-3

Description: Variable Registry

**************************************************/

#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <any>
#include <functional>
#include <sstream>
#include <mutex>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

/**
 * @brief 变量注册器类，用于注册、获取和观察变量值。
 */
class VariableRegistry
{
public:
    /**
     * @brief 观察者 struct，包含观察者名称和回调函数。
     */
    struct Observer
    {
        std::string name;
        std::function<void(const std::string &)> callback;
    };

    /**
     * @brief 注册变量，如果变量已经存在，则返回 false。
     * @tparam T 变量类型，任何可转换为 std::any 类型的类型均可。
     * @param name 变量名称。
     * @return 是否注册成功，如果变量名已经存在，则返回 false。
     */
    template <typename T>
    bool RegisterVariable(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (m_variables.find(name) != m_variables.end())
        {
            return false;
        }

        m_variables[name] = T();
        return true;
    }

    /**
     * @brief 设置指定名称的变量值。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @param value 变量值。
     * @return 是否设置成功，如果变量不存在，则返回 false。
     */
    template <typename T>
    bool SetVariable(const std::string &name, T &&value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (auto it = m_variables.find(name); it != m_variables.end())
        {
            it->second = std::forward<T>(value);
            NotifyObservers(name, value);
            return true;
        }
        return false;
    }

    /**
     * @brief 获取指定名称的变量值。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @return 指定名称的变量值，如果不存在，返回 std::nullopt。
     */
    template <typename T>
    std::optional<T> GetVariable(const std::string &name) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (auto it = m_variables.find(name); it != m_variables.end())
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
        return std::nullopt;
    }

    /**
     * @brief 添加观察者，用于观察变量值的变化。
     * @param name 变量名称。
     * @param observer 观察者 struct。
     */
    void AddObserver(const std::string &name, const Observer &observer)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        m_observers[name].push_back(observer);
    }

    /**
     * @brief 通知指定变量名称的观察者，变量值已经发生变化。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @param value 新的变量值。
     */
    template <typename T>
    void NotifyObservers(const std::string &name, const T &value) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (auto it = m_observers.find(name); it != m_observers.end())
        {
            std::stringstream ss;
            ss << value;
            std::string valueString = ss.str();

            for (const auto &observer : it->second)
            {
                observer.callback(valueString);
            }
        }
    }

    std::unordered_map<std::string, std::any> GetAll() const
    {
        return m_variables;
    }

private:
    /**
     * @brief 所有变量的集合。
     */
    std::unordered_map<std::string, std::any> m_variables;

    /**
     * @brief 观察者的集合，以变量名称为键。
     */
    std::unordered_map<std::string, std::vector<Observer>> m_observers;

    /**
     * @brief 互斥锁，用于保证多线程安全。
     */
    mutable std::mutex m_mutex;
};

static std::string SerializeVariablesToJson(const VariableRegistry &registry)
{
    json j;

    for (const auto entry : registry.GetAll())
    {
        const std::string &name = entry.first;
        const std::any &value = entry.second;

        try
        {
            if (value.type() == typeid(int))
            {
                j[name] = std::any_cast<int>(value);
            }
            else if (value.type() == typeid(double))
            {
                j[name] = std::any_cast<double>(value);
            }
            else if (value.type() == typeid(bool))
            {
                j[name] = std::any_cast<bool>(value);
            }
            else if (value.type() == typeid(std::string))
            {
                j[name] = std::any_cast<std::string>(value);
            }
        }
        catch (const std::bad_any_cast &)
        {
            // 转换失败，忽略该变量或进行其他处理
        }
    }

    return j.dump();
}

/*

int main()
{
    VariableRegistry registry;

    // 注册变量
    if (!registry.RegisterVariable<int>("x"))
    {
        std::cout << "Failed to register variable x." << std::endl;
    }

    if (!registry.RegisterVariable<int>("x"))
    {
        std::cout << "Variable x has already been registered." << std::endl;
    }

    if (registry.SetVariable<int>("x", 10))
    {
        std::cout << "Variable x is set to 10." << std::endl;
    }

    registry.RegisterVariable<std::string>("y");

    // 获取变量值
    if (auto x = registry.GetVariable<int>("x"); x)
    {
        std::cout << "x = " << *x << std::endl;
    }

    // 添加观察者
    registry.AddObserver("x", {"observer1", [](const std::string &value)
                               { std::cout << "observer1: " << value << std::endl; }});
    registry.AddObserver("x", {"observer2", [](const std::string &value)
                               { std::cout << "observer2: " << value << std::endl; }});

    // 更新变量值
    if (registry.SetVariable<int>("x", 20))
    {
        std::cout << "Variable x is set to 20." << std::endl;
    }

    std::string jsonStr = SerializeVariablesToJson(registry);
    std::cout << "Serialized variables: " << jsonStr << std::endl;

    return 0;
}

*/