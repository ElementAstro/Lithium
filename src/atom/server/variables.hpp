/*
 * variables.hpp
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

Date: 2023-12-14

Description: Variable Registry 类，用于注册、获取和观察变量值。

**************************************************/

#pragma once

#include <string>
#include <vector>
#include <any>
#include <functional>
#include <sstream>
#include <mutex>
#include <shared_mutex>
#include <optional>

#ifdef ENALE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

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
     * @param descirption 变量描述。
     * @return 是否注册成功，如果变量名已经存在，则返回 false。
     */
    template <typename T>
    bool RegisterVariable(const std::string &name, const std::string descirption = "");

    /**
     * @brief 设置指定名称的变量值。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @param value 变量值。
     * @return 是否设置成功，如果变量不存在，则返回 false。
     */
    template <typename T>
    bool SetVariable(const std::string &name, T &&value);

    /**
     * @brief 获取指定名称的变量值。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @return 指定名称的变量值，如果不存在，返回 std::nullopt。
     */
    template <typename T>
    std::optional<T> GetVariable(const std::string &name) const;

    /**
     * @brief 判断指定名称的变量是否存在。
     * @param name 变量名称。
     * @return 指定名称的变量是否存在。
     */
    bool HasVariable(const std::string &name) const;

    /**
     * @brief 获取指定名称的变量描述。
     * @param name 变量名称。
     * @return 指定名称的变量描述，如果不存在，返回空字符串。
     */
    std::string GetDescription(const std::string &name) const;

    /**
     * @brief 添加观察者，用于观察变量值的变化。
     * @param name 变量名称。
     * @param observer 观察者 struct。
     */
    void AddObserver(const std::string &name, const Observer &observer);

    /**
     * @brief 通知指定变量名称的观察者，变量值已经发生变化。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @param value 新的变量值。
     */
    template <typename T>
    void NotifyObservers(const std::string &name, const T &value) const;

    /**
     * @brief 移除指定变量名称的观察者。
     * @param name 变量名称。
     * @param observerName 观察者名称。
     * @return 是否移除成功，如果观察者不存在，则返回 false。
     */
    bool RemoveObserver(const std::string &name, const std::string &observerName);

    /**
     * @brief 获取所有变量。
     * @return 所有变量的 map。
     */
#ifdef ENALE_FASTHASH
    emhash8::HashMap<std::string, std::any> GetAll() const;
#else
    std::unordered_map<std::string, std::any> GetAll() const;
#endif

    /**
     * @brief 清空所有变量。
     */
    bool RemoveAll();

    /**
     * @brief 添加获取指定名称变量的回调函数。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @param getter 获取变量值的函数。
     */
    template <typename T>
    void AddGetter(const std::string &name, const std::function<T()> &getter);

    /**
     * @brief 添加检测指定名称变量修改的回调函数。
     * @tparam T 变量类型，需要与注册时相同。
     * @param name 变量名称。
     * @param setter 检测变量修改的函数。
     */
    template <typename T>
    void AddSetter(const std::string &name, const std::function<void(const std::any &)> &setter);

private:
#ifdef ENALE_FASTHASH
    emhash8::HashMap<std::string, std::any> m_variables;
    emhash8::HashMap<std::string, std::string> m_descriptions;
    emhash8::HashMap<std::string, std::vector<Observer>> m_observers;
    emhash8::HashMap<std::string, std::function<std::any()>> m_getters;
    emhash8::HashMap<std::string, std::function<void(const std::any &)>> m_setters;
#else
    /**
     * @brief 所有变量的集合。
     */
    std::unordered_map<std::string, std::any> m_variables;

    /**
     * @brief 所有变量的描述。
     */
    std::unordered_map<std::string, std::string> m_descriptions;

    /**
     * @brief 观察者的集合，以变量名称为键。
     */
    std::unordered_map<std::string, std::vector<Observer>> m_observers;

    /**
     * @brief 获取函数的集合，以变量名称为键。
     */
    std::unordered_map<std::string, std::function<std::any()>> m_getters;

    /**
     * @brief 检测修改函数的集合，以变量名称为键。
     */
    std::unordered_map<std::string, std::function<void(const std::any &)>> m_setters;
#endif

    /**
     * @brief 共享互斥锁，用于保证多线程安全。
     */
    mutable std::shared_mutex m_sharedMutex;
};

template <typename T>
bool VariableRegistry::RegisterVariable(const std::string &name, const std::string descirption)
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);

    if (m_variables.find(name) != m_variables.end())
    {
        return false;
    }

    m_variables[name] = T();
    m_descriptions[name] = descirption;
    return true;
}

template <typename T>
bool VariableRegistry::SetVariable(const std::string &name, T &&value)
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (auto it = m_variables.find(name); it != m_variables.end())
    {
        if (auto setter = m_setters.find(name); setter != m_setters.end())
        {
            setter->second(std::forward<T>(value));
        }
        it->second = std::forward<T>(value);
        NotifyObservers(name, value);
        return true;
    }
    return false;
}

template <typename T>
std::optional<T> VariableRegistry::GetVariable(const std::string &name) const
{
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
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

bool VariableRegistry::HasVariable(const std::string &name) const
{
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    return m_variables.find(name) != m_variables.end();
}

std::string VariableRegistry::GetDescription(const std::string &name) const
{
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);

    if (auto it = m_descriptions.find(name); it != m_descriptions.end())
    {
        return it->second;
    }
    return "";
}

void VariableRegistry::AddObserver(const std::string &name, const Observer &observer)
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (m_observers.find(name) == m_observers.end())
    {
        m_observers[name].push_back(observer);
    }
}

template <typename T>
void VariableRegistry::NotifyObservers(const std::string &name, const T &value) const
{
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    if (!HasVariable(name))
    {
        return;
    }
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

bool VariableRegistry::RemoveObserver(const std::string &name, const std::string &observerName)
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (auto it = m_observers.find(name); it != m_observers.end())
    {
        for (auto it2 = it->second.begin(); it2 != it->second.end(); ++it2)
        {
            if (it2->name == observerName)
            {
                it->second.erase(it2);
                return true;
            }
        }
    }
    return false;
}

std::unordered_map<std::string, std::any> VariableRegistry::GetAll() const
{
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    return m_variables;
}

bool VariableRegistry::RemoveAll()
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    m_variables.clear();
    m_observers.clear();
    return true;
}

template <typename T>
void VariableRegistry::AddGetter(const std::string &name, const std::function<T()> &getter)
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (m_getters.find(name) != m_getters.end())
    {
        return;
    }
    m_getters[name] = getter;
}

template <typename T>
void VariableRegistry::AddSetter(const std::string &name, const std::function<void(const std::any &)> &setter)
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (m_setters.find(name) != m_setters.end())
    {
        return;
    }
    m_setters[name] = setter;
}

static std::string SerializeVariablesToJson(const VariableRegistry &registry)
{
    std::string j = "{";
    for (const auto entry : registry.GetAll())
    {
        const std::string &name = entry.first;
        const std::any &value = entry.second;
        try
        {
            if (value.type() == typeid(int))
            {
                j += "\"" + name + "\":" + std::to_string(std::any_cast<int>(value)) + ",";
            }
            else if (value.type() == typeid(double))
            {
                j += "\"" + name + "\":" + std::to_string(std::any_cast<double>(value)) + ",";
            }
            else if (value.type() == typeid(bool))
            {
                j += "\"" + name + "\":" + std::to_string(std::any_cast<bool>(value)) + ",";
            }
            else if (value.type() == typeid(std::string))
            {
                j += "\"" + name + "\":\"" + std::any_cast<std::string>(value) + "\",";
            }
            else if (value.type() == typeid(std::vector<int>))
            {
                j += "\"" + name + "\":[";
                for (const auto &i : std::any_cast<std::vector<int>>(value))
                {
                    j += std::to_string(i) + ",";
                }
                j.pop_back(); // 删除末尾多余的逗号
                j += "],";
            }
            else if (value.type() == typeid(std::vector<double>))
            {
                j += "\"" + name + "\":[";
                for (const auto &i : std::any_cast<std::vector<double>>(value))
                {
                    j += std::to_string(i) + ",";
                }
                j.pop_back();
                j += "],";
            }
            else if (value.type() == typeid(std::vector<bool>))
            {
                j += "\"" + name + "\":[";
                for (const auto &i : std::any_cast<std::vector<bool>>(value))
                {
                    j += std::to_string(i) + ",";
                }
                j.pop_back();
                j += "],";
            }
            else if (value.type() == typeid(std::vector<std::string>))
            {
                j += "\"" + name + "\":[";
                for (const auto &i : std::any_cast<std::vector<std::string>>(value))
                {
                    j += "\"" + i + "\",";
                }
            }
        }
        catch (const std::bad_any_cast &)
        {
        }
    }
    if (!registry.GetAll().empty())
    {
        j.pop_back(); // 删除末尾多余的逗号
    }

    j += "}";
    return j;
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