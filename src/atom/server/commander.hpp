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
#include <functional>
#include <stack>
#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "atom/type/json.hpp"
using json = nlohmann::json;

template <typename Result, typename Argument>
class CommandDispatcher
{
public:
    /**
     * @brief 定义处理函数类型
     */
    using HandlerFunc = std::function<Result(const Argument &)>;

    /**
     * @brief 注册命令处理函数和撤销函数
     * @param name 命令名称
     * @param handler 命令处理函数
     * @param undoHandler 撤销函数，默认为nullptr
     */
    void RegisterHandler(const std::string &name, const HandlerFunc &handler, const HandlerFunc &undoHandler = nullptr);

    template <class T>
    void RegisterMemberHandler(const std::string &name, T *object, Result (T::*memberFunc)(const Argument &));

    /**
     * @brief 检查是否存在特定名称的命令处理函数
     * @param name 命令名称
     * @return 存在返回true，否则返回false
     */
    bool HasHandler(const std::string &name);

    /**
     * @brief 分派命令并执行处理函数
     * @param name 命令名称
     * @param data 命令参数
     * @return 处理函数的返回结果
     */
    Result Dispatch(const std::string &name, const Argument &data);

    /**
     * @brief 执行撤销操作
     * @return 撤销成功返回true，无可撤销的命令返回false
     */
    bool Undo();

    /**
     * @brief 执行重做操作
     * @return 重做成功返回true，无可重做的命令返回false
     */
    bool Redo();

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::size_t, HandlerFunc> handlers_;
    emhash8::HashMap<std::size_t, HandlerFunc> undoHandlers_;
#else
    std::unordered_map<std::size_t, HandlerFunc> handlers_;
    std::unordered_map<std::size_t, HandlerFunc> undoHandlers_;
#endif

    /**
     * @brief 使用Djb2哈希算法计算字符串的哈希值
     * @param str 输入字符串
     * @return 哈希值
     */
    static std::size_t Djb2Hash(const char *str);

    std::stack<std::pair<std::string, Argument>> commandHistory_;
    std::stack<std::pair<std::string, Argument>> undoneCommands_;
};

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::RegisterHandler(const std::string &name, const HandlerFunc &handler, const HandlerFunc &undoHandler)
{
    auto hash_value = Djb2Hash(name.c_str());
    if (handler)
    {
        handlers_[hash_value] = handler;
    }
    if (undoHandler)
    {
        undoHandlers_[hash_value] = undoHandler;
    }
}

template <typename Result, typename Argument>
template <class T>
void CommandDispatcher<Result, Argument>::RegisterMemberHandler(const std::string &name, T *object, Result (T::*memberFunc)(const Argument &))
{
    auto handler = std::bind(memberFunc, object, std::placeholders::_1);
    RegisterHandler(name, handler);
}

template <typename Result, typename Argument>
bool CommandDispatcher<Result, Argument>::HasHandler(const std::string &name)
{
    return handlers_.find(Djb2Hash(name.c_str())) != handlers_.end();
}

template <typename Result, typename Argument>
Result CommandDispatcher<Result, Argument>::Dispatch(const std::string &name, const Argument &data)
{
    auto it = handlers_.find(Djb2Hash(name.c_str()));
    if (it != handlers_.end())
    {
        return it->second(data);
    }
}

template <typename Result, typename Argument>
std::size_t CommandDispatcher<Result, Argument>::Djb2Hash(const char *str)
{
    std::size_t hash = 5381;
    char c;
    while ((c = *str++) != '\0')
    {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }
    return hash;
}

template <typename Result, typename Argument>
bool CommandDispatcher<Result, Argument>::Undo()
{
    if (commandHistory_.empty())
    {
        return false;
    }
    auto lastCommand = commandHistory_.top();
    commandHistory_.pop();
    undoneCommands_.push(lastCommand);

    auto it = undoHandlers_.find(Djb2Hash(lastCommand.first.c_str()));
    if (it != undoHandlers_.end())
    {
        it->second(lastCommand.second);
    }

    return true;
}

template <typename Result, typename Argument>
bool CommandDispatcher<Result, Argument>::Redo()
{
    if (undoneCommands_.empty())
    {
        return false;
    }
    auto lastUndoneCommand = undoneCommands_.top();
    undoneCommands_.pop();
    commandHistory_.push(lastUndoneCommand);

    auto it = handlers_.find(Djb2Hash(lastUndoneCommand.first.c_str()));
    if (it != handlers_.end())
    {
        it->second(lastUndoneCommand.second);
    }

    return true;
}
