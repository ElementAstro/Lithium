/*
 * connection.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-26

Description: Basic Component Definition

**************************************************/

#pragma once

#define ATOM_COMPONENT_DECLARATION

#include <string>
#include <memory>
#include <vector>

#include "types.hpp"

#include "atom/server/commander.hpp"
#include "atom/server/variables.hpp"
#include "atom/type/args.hpp"
#include "atom/type/ini.hpp"

#define SETVAR_STR(name, value)                              \
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

using ComponentInfo = std::shared_ptr<INIFile>;
using ComponentConfig = std::shared_ptr<INIFile>;

class Component : public std::enable_shared_from_this<Component>
{
public:
    /**
     * @brief Constructs a new Component object.
     */
    explicit Component();

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
    virtual bool Initialize();

    /**
     * @brief Destroys the plugin.
     *
     * @return True if the plugin was destroyed successfully, false otherwise.
     * @note This function is called by the server when the plugin is unloaded.
     * @note This function should be overridden by the plugin.
     * @note The plugin should not be used after this function is called.
     * @note This is for the plugin to release any resources it has allocated.
     */
    virtual bool Destroy();

    std::string GetName() const;

    // -------------------------------------------------------------------
    // Component Infomation methods
    // -------------------------------------------------------------------

    /**
     * @brief Loads the component information from a file.
     *
     * @param path The path to the component information file.
     * @return True if the component information was loaded successfully, false otherwise.
     * @note Usually, the component information file is stored in the plugin's directory.
     */
    bool LoadComponentInfo(const std::string &path);

    template <typename T>
    std::optional<T> getInfo(const std::string section, const std::string &key)
    {
        return m_ComponentInfo->get<T>(section, key);
    }

    std::string getJsonInfo() const;

    std::string getXmlInfo() const;

    template <typename T>
    bool setInfo(const std::string &section, const std::string &key, const T &value)
    {
        return m_ComponentInfo->set(key, value);
    }

    // -------------------------------------------------------------------
    // Component Configuration methods
    // -------------------------------------------------------------------

    /**
     * @brief Loads the component configuration from a file.
     *
     * @param path The path to the component configuration file.
     * @return True if the component configuration was loaded successfully, false otherwise.
     * @note Usually, the component configuration file is stored in the plugin's directory.
     */
    bool LoadComponentConfig(const std::string &path);

    template <typename T>
    std::optional<T> getConfig(const std::string &section, const std::string &key)
    {
        return m_ComponentConfig->get<T>(section, key);
    }

    std::string getJsonConfig() const;

    std::string getXmlConfig() const;

    template <typename T>
    bool setConfig(const std::string &section, const std::string &key, const T &value)
    {
        return m_ComponentConfig->set(section, key, value);
    }

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
    bool RegisterVariable(const std::string &name)
    {
        return m_VariableRegistry->RegisterVariable<T>(name);
    }

    /**
     * @brief Gets the value of the variable with the specified name.
     *
     * @tparam T The type of the variable.
     * @param name The name of the variable.
     * @return An optional containing the value of the variable, or empty if the variable does not exist.
     */
    template <typename T>
    bool SetVariable(const std::string &name, T &&value)
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
    std::optional<T> GetVariable(const std::string &name) const
    {
        return m_VariableRegistry->GetVariable<T>(name);
    }

    /**
     * @brief Gets the information about the variable with the specified name.
     *
     * @param name The name of the variable.
     * @return The information about the variable.
     */
    std::string GetVariableInfo(const std::string &name) const;

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
    void RegisterFunc(const std::string &name, void (ClassType::*handler)(const Args &), ClassType *object);

    /**
     * @brief Gets the information about the function with the specified name.
     *
     * @param name The name of the function.
     * @return The information about the function in JSON format.
     */
    Args GetFuncInfo(const std::string &name);

    /**
     * @brief Runs the function with the specified name and parameters.
     *
     * This function calls a registered member function with the specified name and parameters.
     *
     * @param name The name of the function.
     * @param params The parameters for the function.
     * @return True if the function was executed successfully, false otherwise.
     */
    bool RunFunc(const std::string &name, const Args &params);

    std::function<void(const Args &)> GetFunc(const std::string &name);

private:
    std::string m_name;
    std::string m_ConfigPath;
    std::string m_InfoPath;

    std::unique_ptr<CommandDispatcher<void, Args>> m_CommandDispatcher; ///< The command dispatcher for handling functions.
    std::unique_ptr<VariableRegistry> m_VariableRegistry;               ///< The variable registry for managing variables.

    // Component info in INI format
    ComponentInfo m_ComponentInfo;

    // Component Config in INI format
    ComponentConfig m_ComponentConfig;
};

template <typename ClassType>
void Component::RegisterFunc(const std::string &name, void (ClassType::*handler)(const Args &), ClassType *object)
{
    if (!m_CommandDispatcher->HasHandler(name))
        m_CommandDispatcher->RegisterMemberHandler(name, object, handler);
}
