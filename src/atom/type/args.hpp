/*
 * args.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-28

Description: Argument Container Library for C++

**************************************************/

#pragma once

#ifndef ARG_HPP
#define ARG_HPP

#include <any>
#include <optional>
#include <string>
#include <vector>
#if ENABLE_FASTHASH
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

/**
 * @brief 用于存储和获取参数的容器。
 * @brief A container for storing and retrieving arguments.
 */
class ArgumentContainer
{
public:
    /**
     * @brief 设置参数值。
     * @brief Set the value of a parameter.
     * @tparam T 参数的类型。
     * @tparam T The type of the parameter.
     * @param name 参数的名称。
     * @param name The name of the parameter.
     * @param value 参数的值。
     * @param value The value of the parameter.
     * @note 若参数已存在，则会覆盖原有的值。
     * @note If the parameter exists, it will overwrite the original value.
     */
    template <typename T>
    void set(const std::string &name, const T &value);

    /**
     * @brief 获取参数值。
     * @brief Get the value of a parameter.
     * @tparam T 参数的类型。
     * @tparam T The type of the parameter.
     * @param name 参数的名称。
     * @param name The name of the parameter.
     * @return 参数的值（如果存在），否则返回std::nullopt。
     * @return The value of the parameter (if it exists), otherwise return std::nullopt.
     */
    template <typename T>
    std::optional<T> get(const std::string &name) const;

    /**
     * @brief 移除指定的参数。
     * @brief Remove a specified parameter.
     * @param name 要移除的参数的名称。
     * @param name The name of the parameter to be removed.
     * @return 如果成功移除参数，则返回true；否则返回false
     * @return If the parameter is successfully removed, return true; otherwise return false。
     */
    bool remove(const std::string &name);

    /**
     * @brief 检查参数是否存在。
     * @brief Check if a parameter exists.
     * @param name 要检查的参数的名称。
     * @param name The name of the parameter to be checked.
     * @return 如果参数存在，则返回true；否则返回false。
     * @return If the parameter exists, return true; otherwise return false.
     */
    bool contains(const std::string &name) const;

    /**
     * @brief 获取参数的数量。
     * @brief Get the number of parameters.
     * @return 参数的数量。
     * @return The number of parameters.
     */
    std::size_t size() const;

    /**
     * @brief 获取所有参数的名称。
     * @brief Get all parameter names.
     * @return 包含所有参数名称的字符串向量。
     * @return A vector containing all parameter names.
     */
    std::vector<std::string> getNames() const;

    /**
     * @brief 重载索引运算符[]以获取和设置参数值。
     * @brief Overload the index operator [] to get and set the value of a parameter.
     * @tparam T 参数的类型。
     * @tparam T The type of the parameter.
     * @param name 参数的名称。
     * @param name The name of the parameter.
     * @return 参数的引用。
     * @return A reference to the parameter.
     */
    template <typename T>
    T &operator[](const std::string &name)
    {
        return std::any_cast<T &>(m_arguments[name]);
    }

    /**
     * @brief 重载赋值运算符=以设置参数值。
     * @brief Overload the assignment operator = to set the value of a parameter.
     * @tparam T 参数的类型。
     * @tparam T The type of the parameter.
     * @param argument 要设置的参数（名称和值）。
     * @param argument The parameter to be set (name and value).
     */
    template <typename T>
    void operator=(const std::pair<std::string, T> &argument)
    {
        set(argument.first, argument.second);
    }

    /**
     * @brief 重载赋值运算符=以设置参数值。
     * @brief Overload the assignment operator = to set the value of a parameter.
     * @param container 要设置的参数容器。
     * @param container The parameter container to be set.
     */
#if ENABLE_FASTHASH
    void operator=(const emhash8::HashMap<std::string, std::any> &container)
#else
    void operator=(const std::unordered_map<std::string, std::any> &container)
#endif
    {
        m_arguments = container;
    }

    std::string toJson() const;

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, std::any> m_arguments; // 存储参数的容器
#else
    std::unordered_map<std::string, std::any> m_arguments; // 存储参数的容器
#endif
};

template <typename T>
void ArgumentContainer::set(const std::string &name, const T &value)
{
    if (m_arguments.find(name) != m_arguments.end())
    {
        m_arguments.erase(name);
    }
    m_arguments[name] = value;
}

template <typename T>
std::optional<T> ArgumentContainer::get(const std::string &name) const
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


using Args = ArgumentContainer;

#endif