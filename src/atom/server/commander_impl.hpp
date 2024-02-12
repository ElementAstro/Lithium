/*
 * commander_impl.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Commander

**************************************************/

#ifndef ATOM_SERVER_COMMANDER_IMPL_HPP
#define ATOM_SERVER_COMMANDER_IMPL_HPP

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

#endif