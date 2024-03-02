/*
 * commander.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-6-17

Description: Commander

**************************************************/

#ifndef ATOM_SERVER_COMMANDER_HPP
#define ATOM_SERVER_COMMANDER_HPP

#include <functional>
#include <mutex>
#include <shared_mutex>
#include <stack>
#include <string>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

/**
 * @brief Generic command dispatcher class for handling and dispatching
 * commands.
 *
 * This class allows registration of handler functions for specific commands,
 * along with optional undo handlers and decorators to modify or enhance the
 * behavior of the registered handlers. It also provides functionality for
 * dispatching commands, undoing and redoing commands, and managing function
 * descriptions and command history.
 *
 * @tparam Result The result type of the command handler function.
 * @tparam Argument The argument type of the command handler function.
 */
template <typename Result, typename Argument>
class CommandDispatcher {
public:
    using HandlerFunc = std::function<Result(const Argument &)>;
    using DecoratorFunc = std::shared_ptr<decorator<HandlerFunc>>;

    /**
     * @brief Registers a handler function for a specific command.
     *
     * @param name The name of the command.
     * @param handler The handler function for the command.
     * @param undoHandler Optional undo handler function for the command.
     */
    void registerHandler(const std::string &name, const HandlerFunc &handler,
                         const HandlerFunc &undoHandler = nullptr);

    /**
     * @brief Registers a member function handler for a specific command.
     *
     * @tparam T The type of the object.
     * @param name The name of the command.
     * @param object The object.
     * @param memberFunc The member function of the object.
     * @param undoHandler Optional undo handler function for the command.
     */
    template <class T>
    void registerMemberHandler(const std::string &name, T *object,
                               Result (T::*memberFunc)(const Argument &));

    void registerDecorator(const std::string &name,
                           const DecoratorFunc &decorator);

    HandlerFunc getHandler(const std::string &name);

    bool hasHandler(const std::string &name);

    Result dispatch(const std::string &name, const Argument &data);

    bool undo();

    bool redo();

    bool removeAll();

    void registerFunctionDescription(const std::string &name,
                                     const std::string &description);

    std::string getFunctionDescription(const std::string &name);

    void removeFunctionDescription(const std::string &name);

    void clearFunctionDescriptions();

    void setMaxHistorySize(size_t maxSize);

    size_t getMaxHistorySize() const;

private:
#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, HandlerFunc> m_handlers;
    emhash8::HashMap<std::string, DecoratorFunc> m_decorators;
    emhash8::HashMap<std::string, HandlerFunc> m_undoHandlers;
    emhash8::HashMap<std::string, std::string> m_descriptions;
#else
    std::unordered_map<std::string, HandlerFunc> m_handlers;
    std::unordered_map<std::string, DecoratorFunc> m_decorators;
    std::unordered_map<std::string, HandlerFunc> m_undoHandlers;
    std::unordered_map<std::string, std::string> m_descriptions;
#endif

    std::stack<std::pair<std::string, Argument>> m_commandHistory;
    std::stack<std::pair<std::string, Argument>> m_undoneCommands;

    mutable std::shared_mutex m_sharedMutex;
    size_t m_maxHistorySize = 100;
};

#include "commander_impl.hpp"

#endif
