/*
 * component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-26

Description: Basic Component Definition

**************************************************/

// Max: Obviously, directly use json.hpp is much simpler than self-implement type

#ifndef ATOM_COMPONENT_HPP
#define ATOM_COMPONENT_HPP

#include <memory>
#include <vector>

#include "types.hpp"

#include "atom/server/commander.hpp"
#include "atom/server/variables.hpp"

#include "atom/type/json.hpp"
using json = nlohmann::json;

#define SETVAR_STR(name, value)                         \
    m_VariableRegistry->RegisterVariable<std::string>(name); \
    m_VariableRegistry->SetVariable(name, value);

#define SETVAR_INT(name, value)                      \
    m_VariableRegistry->RegisterVariable<int>(name); \
    m_VariableRegistry->SetVariable(name, value);

#define SETVAR_BOOL(name, value)                      \
    m_VariableRegistry->RegisterVariable<bool>(name); \
    m_VariableRegistry->SetVariable(name, value);

#define SETVAR_DOUBLE(name, value)                      \
    m_VariableRegistry->RegisterVariable<double>(name); \
    m_VariableRegistry->SetVariable(name, value);

class Component : public std::enable_shared_from_this<Component>
{
public:
    /**
     * @brief Constructs a new Component object.
     */
    explicit Component(const std::string &name);

    /**
     * @brief Destroys the Component object.
     */
    virtual ~Component();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    /**
     * @brief Initializes the plugin.
     *
     * @return True if the plugin was initialized successfully, false otherwise.
     * @note This function is called by the server when the plugin is loaded.
     * @note This function should be overridden by the plugin.
     */
    virtual bool initialize();

    /**
     * @brief Destroys the plugin.
     *
     * @return True if the plugin was destroyed successfully, false otherwise.
     * @note This function is called by the server when the plugin is unloaded.
     * @note This function should be overridden by the plugin.
     * @note The plugin should not be used after this function is called.
     * @note This is for the plugin to release any resources it has allocated.
     */
    virtual bool destroy();

    std::string getName() const;

    // -------------------------------------------------------------------
    // Component Configuration methods
    // -------------------------------------------------------------------

    // Max: Should we use JSON here?
    // 2024/02/13 Max: Obviously, directly use json.hpp is much simpler than self-implement type

    /**
     * @brief Loads the component configuration from a file.
     *
     * @param path The path to the component configuration file.
     * @return True if the component configuration was loaded successfully, false otherwise.
     * @note Usually, the component configuration file is stored in the plugin's directory.
     */
    bool loadConfig(const std::string &path);

    bool saveConfig();

    json getValue(const std::string &key_path) const;

    // -------------------------------------------------------------------
    // Variable methods
    // -------------------------------------------------------------------

    /**
     * @brief Registers a member function with a specific name and handler.
     *
     * This function allows plugins to register member functions that can be called by other plugins.
     *
     * @tparam ClassType The type of the class that owns the handler function.
     * @param name The name of the function.
     * @param handler The handler function for the function.
     * @param object The object instance that owns the handler function.
     */
    template <typename T>
    bool registerVariable(const std::string &name, const T &value, const std::string &description = "")
    {
        return m_VariableRegistry->RegisterVariable<T>(name, value, description);
    }

    /**
     * @brief Gets the value of the variable with the specified name.
     *
     * @tparam T The type of the variable.
     * @param name The name of the variable.
     * @return An optional containing the value of the variable, or empty if the variable does not exist.
     */
    template <typename T>
    bool setVariable(const std::string &name, const T &value)
    {
        return m_VariableRegistry->SetVariable<T>(name, value);
    }

    /**
     * @brief Gets the value of the variable with the specified name.
     *
     * @tparam T The type of the variable.
     * @param name The name of the variable.
     * @return An optional containing the value of the variable, or empty if the variable does not exist
     * @note When you try to get a variable that does not exist, an exception will be thrown
     * @note This function is thread-safe
     * @note This function is for the server to get the value of a variable of the plugin.
     */
    template <typename T>
    [[nodiscard]] std::optional<T> getVariable(const std::string &name) const
    {
        return m_VariableRegistry->GetVariable<T>(name);
    }

    /**
     * @brief Gets the information about the variable with the specified name.
     *
     * @param name The name of the variable.
     * @return The information about the variable.
     */
    std::string getVariableInfo(const std::string &name) const;

    // -------------------------------------------------------------------
    // Function methods
    // -------------------------------------------------------------------

    /**
     * @brief Sets the value of the variable with the specified name.
     *
     * @tparam T The type of the variable.
     * @param name The name of the variable.
     * @param value The value to set.
     * @return True if the variable was set successfully, false otherwise.
     */
    template <typename ClassType>
    void registerFunc(const std::string &name, json (ClassType::*handler)(const json &), ClassType *object);

    /**
     * @brief Gets the information about the function with the specified name.
     *
     * @param name The name of the function.
     * @return The information about the function in JSON format.
     */
    json getFuncInfo(const std::string &name);

    /**
     * @brief Runs the function with the specified name and parameters.
     *
     * This function calls a registered member function with the specified name and parameters.
     *
     * @param name The name of the function.
     * @param params The parameters for the function.
     * @return True if the function was executed successfully, false otherwise.
     */
    bool runFunc(const std::string &name, const json &params);

    std::function<json(const json &)> getFunc(const std::string &name);

    json createSuccessResponse(const std::string &command, const json &value);

    json createErrorResponse(const std::string &command, const json &error, const std::string &message);

    json createWarningResponse(const std::string &command, const json &warning, const std::string &message);

private:
    std::string m_name;
    std::string m_ConfigPath;
    std::string m_InfoPath;

    std::unique_ptr<CommandDispatcher<json, json>> m_CommandDispatcher; ///< The command dispatcher for handling functions.
    std::unique_ptr<VariableRegistry> m_VariableRegistry;               ///< The variable registry for managing variables.

    json m_config;
    std::mutex m_mutex;
};

template <typename ClassType>
void Component::registerFunc(const std::string &name, json (ClassType::*handler)(const json &), ClassType *object)
{
    if (!m_CommandDispatcher->HasHandler(name))
        m_CommandDispatcher->RegisterMemberHandler(name, object, handler);
}

#endif
