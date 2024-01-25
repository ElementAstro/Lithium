/*
 * commander.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Commander

**************************************************/

#pragma once

#include <string>
#include <functional>
#include <stack>
#include <shared_mutex>
#include <mutex>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

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

    /**
     * @brief 注册成员函数处理函数
     * @param name 命令名称
     * @param object 命令处理函数所属对象
     * @param memberFunc 命令处理函数
     */
    template <class T>
    void RegisterMemberHandler(const std::string &name, T *object, Result (T::*memberFunc)(const Argument &));

    /**
     * @brief 获取特定名称的命令处理函数
     * @param name 命令名称
     * @return 命令处理函数
     */
    std::function<Result(const Argument &)> GetHandler(const std::string &name);

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

    /**
     * @brief 清空所有命令处理函数
     */
    bool RemoveAll();

    /**
     * @brief 注册函数的描述信息
     * @param name 函数名称
     * @param description 函数描述信息
     */
    void RegisterFunctionDescription(const std::string &name, const std::string &description);

    /**
     * @brief 获取函数的描述信息
     * @param name 函数名称
     * @return 函数描述信息
     */
    std::string GetFunctionDescription(const std::string &name);

    /** @brief 删除函数的描述信息 */
    void RemoveFunctionDescription(const std::string &name);

    /** @brief 清空函数的描述信息 */
    void ClearFunctionDescriptions();

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, HandlerFunc> handlers_;
    emhash8::HashMap<std::string, HandlerFunc> undoHandlers_;
    emhash8::HashMap<std::string, std::string> descriptions_;
#else
    std::unordered_map<std::string, HandlerFunc> handlers_;
    std::unordered_map<std::string, HandlerFunc> undoHandlers_;
    std::unordered_map<std::string, std::string> descriptions_;
#endif

    std::stack<std::pair<std::string, Argument>> commandHistory_;
    std::stack<std::pair<std::string, Argument>> undoneCommands_;

    mutable std::shared_mutex m_sharedMutex;
};

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::RegisterHandler(const std::string &name, const HandlerFunc &handler, const HandlerFunc &undoHandler)
{
    if (name.empty())
    {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (handler)
    {
        handlers_[name] = handler;
    }
    if (undoHandler)
    {
        undoHandlers_[name] = undoHandler;
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
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    return handlers_.find(name) != handlers_.end();
}

template <typename Result, typename Argument>
std::function<Result(const Argument &)> CommandDispatcher<Result, Argument>::GetHandler(const std::string &name)
{
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    auto it = handlers_.find(name);
    if (it != handlers_.end())
    {
        return it->second;
    }
    return nullptr;
}

template <typename Result, typename Argument>
Result CommandDispatcher<Result, Argument>::Dispatch(const std::string &name, const Argument &data)
{
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    auto it = handlers_.find(name);
    if (it != handlers_.end())
    {
        return it->second(data);
    }
    return Result();
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

template <typename Result, typename Argument>
bool CommandDispatcher<Result, Argument>::RemoveAll()
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    handlers_.clear();
    undoHandlers_.clear();
    descriptions_.clear();
    while (!commandHistory_.empty())
    {
        commandHistory_.pop();
    }
    return true;
}

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::RegisterFunctionDescription(const std::string &name, const std::string &description)
{
    if (name.empty() || description.empty())
    {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    descriptions_[name] = description;
}

template <typename Result, typename Argument>
std::string CommandDispatcher<Result, Argument>::GetFunctionDescription(const std::string &name)
{
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    auto it = descriptions_.find(name);
    if (it != descriptions_.end())
    {
        return it->second;
    }
    return "";
}

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::RemoveFunctionDescription(const std::string &name)
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (name.empty())
    {
        return;
    }
    descriptions_.erase(name);
}

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::ClearFunctionDescriptions()
{
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    descriptions_.clear();
}