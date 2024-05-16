/*
 * component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-26

Description: Basic Component Definition

**************************************************/

#ifndef ATOM_COMPONENT_HPP
#define ATOM_COMPONENT_HPP

#include <memory>
#include <shared_mutex>
#include <vector>

#include "types.hpp"

#include "dispatch.hpp"
#include "var.hpp"

#include "atom/function/type_info.hpp"
#include "atom/type/noncopyable.hpp"

class Component : public std::enable_shared_from_this<Component> {
public:
    /**
     * @brief Constructs a new Component object.
     */
    explicit Component(const std::string& name);

    /**
     * @brief Destroys the Component object.
     */
    virtual ~Component();

    // -------------------------------------------------------------------
    // Inject methods
    // -------------------------------------------------------------------

    std::weak_ptr<const Component> getInstance() const;

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

    /**
     * @brief Gets the name of the plugin.
     *
     * @return The name of the plugin.
     */
    std::string getName() const;

    /**
     * @brief Gets the type information of the plugin.
     *
     * @return The type information of the plugin.
     */
    Type_Info getTypeInfo() const;

    // -------------------------------------------------------------------
    // Variable methods
    // -------------------------------------------------------------------

    template <typename T>
    void addVariable(const std::string& name, T initialValue,
                     const std::string& description = "",
                     const std::string& alias = "",
                     const std::string& group = "") {
        m_VariableManager->addVariable(name, initialValue, description, alias,
                                       group);
    }

    template <typename T>
    void setRange(const std::string& name, T min, T max) {
        m_VariableManager->setRange(name, min, max);
    }

    void setStringOptions(const std::string& name,
                          std::vector<std::string> options) {
        m_VariableManager->setStringOptions(name, options);
    }

    /**
     * @brief Gets a variable by name.
     * @param name The name of the variable.
     * @return A shared pointer to the variable.
     */
    template <typename T>
    std::shared_ptr<Trackable<T>> getVariable(const std::string& name) {
        return m_VariableManager->getVariable<T>(name);
    }

    /**
     * @brief Sets the value of a variable.
     * @param name The name of the variable.
     * @param newValue The new value of the variable.
     * @note const char * is not equivalent to std::string, please use
     * std::string
     */
    template <typename T>
    void setValue(const std::string& name, T newValue) {
        m_VariableManager->setValue(name, newValue);
    }

    // -------------------------------------------------------------------
    // Function methods
    // -------------------------------------------------------------------

    template <typename Callable>
    void def(const std::string& name, Callable&& func,
             const std::string& group = "",
             const std::string& description = "");

    template <typename Ret>
    void def(const std::string& name, Ret (*func)(),
             const std::string& group = "",
             const std::string& description = "");

    template <typename... Args, typename Ret>
    void def(const std::string& name, Ret (*func)(Args...),
             const std::string& group = "",
             const std::string& description = "");

    template <typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(),
             const PointerSentinel<Class>& instance,
             const std::string& group = "",
             const std::string& description = "");

    template <typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(),
             std::shared_ptr<Class> instance, const std::string& group = "",
             const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(Args...),
             std::shared_ptr<Class> instance, const std::string& group = "",
             const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(Args...) const,
             std::shared_ptr<Class> instance, const std::string& group = "",
             const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(Args...),
             const PointerSentinel<Class>& instance,
             const std::string& group = "",
             const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(Args...) const,
             const PointerSentinel<Class>& instance,
             const std::string& group = "",
             const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(Args...) noexcept,
             const PointerSentinel<Class>& instance,
             const std::string& group = "",
             const std::string& description = "");

    void addAlias(const std::string& name, const std::string& alias);

    void addGroup(const std::string& name, const std::string& group);

    void setTimeout(const std::string& name, std::chrono::milliseconds timeout);

    template <typename... Args>
    std::any dispatch(const std::string& name, Args&&... args) {
        return m_CommandDispatcher->dispatch(name, std::forward<Args>(args)...);
    }

    std::any dispatch(const std::string& name,
                      const std::vector<std::any>& args) {
        return m_CommandDispatcher->dispatch(name, args);
    }

    [[nodiscard]] bool has(const std::string& name) const;

    void clearCache();

    void removeCommand(const std::string& name);

    std::vector<std::string> getCommandsInGroup(const std::string& group) const;

    std::string getCommandDescription(const std::string& name) const;

#if ENABLE_FASTHASH
    emhash::HashSet<std::string> getCommandAliases(
        const std::string& name) const;
#else
    std::unordered_set<std::string> getCommandAliases(
        const std::string& name) const;
#endif

    std::vector<std::string> getAllCommands() const;

    // -------------------------------------------------------------------
    // Other Components methods
    // -------------------------------------------------------------------
    /**
     * @note This method is not thread-safe. And we must make sure the pointer
     * is valid. The PointerSentinel will help you to avoid this problem. We
     * will directly get the std::weak_ptr from the pointer.
     */

    /**
     * @return The names of the components that are needed by this component.
     * @note This will be called when the component is initialized.
     */
    std::vector<std::string> getNeededComponents() const;

    void addOtherComponent(const std::string& name,
                           const std::weak_ptr<Component>& component);

    void removeOtherComponent(const std::string& name);

    void clearOtherComponents();

    std::weak_ptr<Component> getOtherComponent(const std::string& name);

private:
    std::string m_name;
    std::string m_configPath;
    std::string m_infoPath;
    Type_Info m_typeInfo;

    std::shared_ptr<CommandDispatcher>
        m_CommandDispatcher;  ///< The command dispatcher for managing commands.
    std::shared_ptr<VariableManager>
        m_VariableManager;  ///< The variable registry for managing variables.

    std::unordered_map<std::string, std::weak_ptr<Component>> m_OtherComponents;
};

#include "component.inl"

#endif
