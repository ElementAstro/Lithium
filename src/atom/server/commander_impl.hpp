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
CommandDispatcher<Result, Argument>::~CommandDispatcher() {
    m_handlers.clear();
    m_decorators.clear();
    m_undoHandlers.clear();
    m_descriptions.clear();
}

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::registerHandler(
    const std::string &name, const HandlerFunc &handler,
    const HandlerFunc &undoHandler) {
    if (name.empty()) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (handler) {
        m_handlers[name] = handler;
    }
    if (undoHandler) {
        m_undoHandlers[name] = undoHandler;
    }
}

template <typename Result, typename Argument>
template <class T>
void CommandDispatcher<Result, Argument>::registerMemberHandler(
    const std::string &name, T *object,
    Result (T::*memberFunc)(const Argument &)) {
    auto handler = std::bind(memberFunc, object, std::placeholders::_1);
    registerHandler(name, handler);
}

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::registerDecorator(
    const std::string &name, const DecoratorFunc &decorator) {
    if (name.empty() || !decorator) {
        return;
    }
    m_decorators[name] = decorator;
}

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::registerLoopDecorator(
    const std::string &name, const LoopDecoratorFunc &decorator) {
    if (name.empty() || !decorator) {
        return;
    }
    m_decorators[name] = decorator;
}

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::registerConditionalDecorator(
    const std::string &name, const ConditionalDecoratorFunc &decorator) {
    if (name.empty() || !decorator) {
        return;
    }
    m_decorators[name] = decorator;
}

template <typename Result, typename Argument>
bool CommandDispatcher<Result, Argument>::hasHandler(const std::string &name) {
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    return m_handlers.find(name) != m_handlers.end();
}

template <typename Result, typename Argument>
typename CommandDispatcher<Result, Argument>::HandlerFunc
CommandDispatcher<Result, Argument>::getHandler(const std::string &name) {
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    auto it = m_handlers.find(name);
    if (it != m_handlers.end()) {
        return it->second;
    }
    return nullptr;
}

template <typename Result, typename Argument>
Result CommandDispatcher<Result, Argument>::dispatch(const std::string &name,
                                                     const Argument &data) {
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);

    // Max: Here we check if a decorator is registered for the command and run it in advance
    // Check if a decorator is registered for the command
    auto it = m_decorators.find(name);
    if (it != m_decorators.end()) {
        auto decorator = it->second;
        return decorator->operator()(data);
    }

    // If no decorator found, proceed with the original handler
    auto handlerIt = m_handlers.find(name);
    if (handlerIt != m_handlers.end()) {
        return handlerIt->second(data);
    }

    return Result();  // Default return if no handler or decorator found
}

template <typename Result, typename Argument>
bool CommandDispatcher<Result, Argument>::undo() {
    if (m_commandHistory.empty()) {
        return false;
    }
    auto lastCommand = m_commandHistory.top();
    m_commandHistory.pop();
    m_undoneCommands.push(lastCommand);

    auto it = m_undoHandlers.find(lastCommand.first);
    if (it != m_undoHandlers.end()) {
        it->second(lastCommand.second);
    }

    return true;
}

template <typename Result, typename Argument>
bool CommandDispatcher<Result, Argument>::redo() {
    if (m_undoneCommands.empty()) {
        return false;
    }
    auto lastUndoneCommand = m_undoneCommands.top();
    m_undoneCommands.pop();
    m_commandHistory.push(lastUndoneCommand);

    auto it = m_handlers.find(lastUndoneCommand.first);
    if (it != m_handlers.end()) {
        it->second(lastUndoneCommand.second);
    }

    return true;
}

template <typename Result, typename Argument>
bool CommandDispatcher<Result, Argument>::removeAll() {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    m_handlers.clear();
    m_undoHandlers.clear();
    m_descriptions.clear();
    while (!m_commandHistory.empty()) {
        m_commandHistory.pop();
    }
    return true;
}

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::registerFunctionDescription(
    const std::string &name, const std::string &description) {
    if (name.empty() || description.empty()) {
        return;
    }
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    m_descriptions[name] = description;
}

template <typename Result, typename Argument>
std::string CommandDispatcher<Result, Argument>::getFunctionDescription(
    const std::string &name) {
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    auto it = m_descriptions.find(name);
    if (it != m_descriptions.end()) {
        return it->second;
    }
    return "";
}

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::removeFunctionDescription(
    const std::string &name) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    if (name.empty()) {
        return;
    }
    m_descriptions.erase(name);
}

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::clearFunctionDescriptions() {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    m_descriptions.clear();
}

template <typename Result, typename Argument>
void CommandDispatcher<Result, Argument>::setMaxHistorySize(size_t maxSize) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    m_maxHistorySize = maxSize;
}

template <typename Result, typename Argument>
size_t CommandDispatcher<Result, Argument>::getMaxHistorySize() const {
    std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
    return m_maxHistorySize;
}

#endif