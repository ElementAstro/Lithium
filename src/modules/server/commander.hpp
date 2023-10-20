/*
 * commander.hpp
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

Date: 2023-6-17

Description: Commander

**************************************************/

#pragma once

#include <string>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif
#include <functional>

#include "nlohmann/json.hpp"
using json = nlohmann::json;

/**
 * @brief 类 CommandDispatcher 负责命令的派发和处理。
 */
class CommandDispatcher
{
public:
    /**
     * @brief HandlerFunc 是用于处理命令的函数类型。
     *
     * 该函数应该接受一个 `json` 类型的参数，表示命令所携带的数据。
     */
    using HandlerFunc = std::function<const json(const json &)>;

    /**
     * @brief RegisterHandler 函数用于将一个命令处理程序注册到 `CommandDispatcher` 中。
     *
     * @tparam ClassType 命令处理程序所属的类类型。
     * @param name 命令的名称。
     * @param handler 处理命令的成员函数指针。
     * @param instance 处理命令的对象指针。
     */
    template <typename ClassType>
    void RegisterHandler(const std::string &name, const json (ClassType::*handler)(const json &), ClassType *instance)
    {
        auto hash_value = Djb2Hash(name.c_str());
        handlers_[hash_value] = std::bind(handler, instance, std::placeholders::_1);
    }

    /**
     * @brief HasHandler 函数用于检查是否有名为 `name` 的命令处理程序。
     *
     * @param name 要检查的命令名称。
     * @return 如果存在名为 `name` 的命令处理程序，则返回 `true`；否则返回 `false`。
     */
    bool HasHandler(const std::string &name);

    /**
     * @brief Dispatch 函数用于派发一个命令，并将它交给相应的处理程序处理。
     *
     * @param name 要派发的命令的名称。
     * @param data 命令所携带的数据。
     */
    json Dispatch(const std::string &name, const json &data);

private:
    /**
     * @brief handlers_ 是一个哈希表，存储了所有已注册的命令处理程序。
     *
     * 键值为哈希值，值为命令处理程序本身。
     */
#if ENABLE_FASTHASH
    emhash8::HashMap<std::size_t, HandlerFunc> handlers_;
#else
    std::unordered_map<std::size_t, HandlerFunc> handlers_;
#endif

    /**
     * @brief Djb2Hash 函数是一个字符串哈希函数，用于将字符串转换成哈希值。
     *
     * @param str 要转换的字符串。
     * @return 转换后的哈希值。
     */
    static std::size_t Djb2Hash(const char *str);
};

/**
 * @brief 类 VCommandDispatcher 负责命令的派发和处理。
 */
class VCommandDispatcher
{
public:
    /**
     * @brief HandlerFunc 是用于处理命令的函数类型。
     *
     * 该函数应该接受一个 `json` 类型的参数，表示命令所携带的数据。
     */
    using HandlerFunc = std::function<void(const json &)>;

    /**
     * @brief RegisterHandler 函数用于将一个命令处理程序注册到 `CommandDispatcher` 中。
     *
     * @tparam ClassType 命令处理程序所属的类类型。
     * @param name 命令的名称。
     * @param handler 处理命令的成员函数指针。
     * @param instance 处理命令的对象指针。
     */
    template <typename ClassType>
    void RegisterHandler(const std::string &name, void (ClassType::*handler)(const json &), ClassType *instance)
    {
        auto hash_value = Djb2Hash(name.c_str());
        handlers_[hash_value] = std::bind(handler, instance, std::placeholders::_1);
    }

    /**
     * @brief HasHandler 函数用于检查是否有名为 `name` 的命令处理程序。
     *
     * @param name 要检查的命令名称。
     * @return 如果存在名为 `name` 的命令处理程序，则返回 `true`；否则返回 `false`。
     */
    bool HasHandler(const std::string &name);

    /**
     * @brief Dispatch 函数用于派发一个命令，并将它交给相应的处理程序处理。
     *
     * @param name 要派发的命令的名称。
     * @param data 命令所携带的数据。
     */
    void Dispatch(const std::string &name, const json &data);

private:
    /**
     * @brief handlers_ 是一个哈希表，存储了所有已注册的命令处理程序。
     *
     * 键值为哈希值，值为命令处理程序本身。
     */
#if ENABLE_FASTHASH
    emhash8::HashMap<std::size_t, HandlerFunc> handlers_;
#else
    std::unordered_map<std::size_t, HandlerFunc> handlers_;
#endif

    /**
     * @brief Djb2Hash 函数是一个字符串哈希函数，用于将字符串转换成哈希值。
     *
     * @param str 要转换的字符串。
     * @return 转换后的哈希值。
     */
    static std::size_t Djb2Hash(const char *str);
};
