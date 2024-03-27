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

#include "atom/experiment/decorate.hpp"
#include "atom/experiment/noncopyable.hpp"

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
class CommandDispatcher : public NonCopyable {
public:
    using HandlerFunc = std::function<Result(const Argument &)>;
    using DecoratorFunc = std::shared_ptr<decorator<HandlerFunc>>;
    using LoopDecoratorFunc = std::shared_ptr<LoopDecorator<HandlerFunc>>;
    using ConditionalDecoratorFunc =
        std::shared_ptr<ConditionCheckDecorator<HandlerFunc>>;

    CommandDispatcher() = default;
    ~CommandDispatcher();

    /**
     * @brief Creates a shared pointer to a CommandDispatcher instance.
     *
     * @return std::shared_ptr<CommandDispatcher> A shared pointer to a
     * CommandDispatcher instance.
     */
    static std::shared_ptr<CommandDispatcher> createShared() {
        return std::make_shared<CommandDispatcher>();
    }

    /**
     * @brief Creates a unique pointer to a CommandDispatcher instance.
     *
     * @return std::unique_ptr<CommandDispatcher> A unique pointer to a
     * CommandDispatcher instance.
     */
    static std::unique_ptr<CommandDispatcher> createUnique() {
        return std::make_unique<CommandDispatcher>();
    }

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

    /**
     * @brief Registers a decorator for a specific command.
     *
     * @param name The name of the command.
     * @param decorator The decorator function for the command.
     */
    void registerDecorator(const std::string &name,
                           const DecoratorFunc &decorator);

    /**
     * @brief Registers a loop decorator for a specific command.
     *
     * @param name The name of the command.
     * @param decorator The decorator function for the command.
     */
    void registerLoopDecorator(const std::string &name,
                               const LoopDecoratorFunc &decorator);

    /**
     * @brief Registers a conditional decorator for a specific command.
     *
     * @param name The name of the command.
     * @param decorator The decorator function for the command.
     */
    void registerConditionalDecorator(const std::string &name,
                                    const ConditionalDecoratorFunc &decorator);

    /**
     * @brief Returns the handler function for a specific command.
     *
     * @param name The name of the command.
     * @return The handler function for the command.
     */
    HandlerFunc getHandler(const std::string &name);

    /**
     * @brief Returns whether a handler function is registered for a specific
     * command.
     *
     * @param name The name of the command.
     * @return Whether a handler function is registered for the command.
     */
    bool hasHandler(const std::string &name);

    /**
     * @brief Dispatches a command.
     *
     * @param name The name of the command.
     * @param data The data to pass to the command handler.
     * @return The result of the command handler.
     */
    Result dispatch(const std::string &name, const Argument &data);
    
    // Max: Redo and Undo system is not ready to be used.

    /**
     * @brief Undoes the last dispatched command.
     *
     * @return Whether the command was successfully undone.
     */
    bool undo();

    /**
     * @brief Redoes the last undone command.
     *
     * @return Whether the command was successfully redone.
     */
    bool redo();

    /**
     * @brief Removes all registered handler functions.
     *
     * @return Whether the handler functions were successfully removed.
     */
    bool removeAll();

    /**
     * @brief Registers a description for a specific command.
     *
     * @param name The name of the command.
     * @param description The description of the command.
     */
    void registerFunctionDescription(const std::string &name,
                                     const std::string &description);

    /**
     * @brief Returns the description of a specific command.
     *
     * @param name The name of the command.
     * @return The description of the command.
     */
    std::string getFunctionDescription(const std::string &name);

    /**
     * @brief Removes the description of a specific command.
     *
     * @param name The name of the command.
     */
    void removeFunctionDescription(const std::string &name);

    /**
     * @brief Removes all registered descriptions.
     *
     * @return Whether the descriptions were successfully removed.
     */
    void clearFunctionDescriptions();

    /**
     * @brief Sets the maximum number of commands that can be stored in the
     * command history.
     *
     * @param maxSize The maximum number of commands that can be stored in the
     * command history.
     */
    void setMaxHistorySize(size_t maxSize);

    /**
     * @brief Returns the maximum number of commands that can be stored in the
     * command history.
     *
     * @return The maximum number of commands that can be stored in the command
     * history.
     */
    size_t getMaxHistorySize() const;

private:
    // Max: Emhash8 is used to speed up the command dispatcher.
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
