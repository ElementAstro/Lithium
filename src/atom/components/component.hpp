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

#include "configor.hpp"

#include "atom/function/type_info.hpp"
#include "atom/type/noncopyable.hpp"

template <typename Delivery>
class Component : public std::enable_shared_from_this<Delivery>,
                  public NonCopyable {
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

    std::unordered_map<std::string, bool> getComponentAbilities() const;

    bool hasAbility(const std::string& ability) const;

    /**
     * @brief Gets the name of the plugin.
     *
     * @return The name of the plugin.
     */
    std::string getName() const;

    Type_Info getTypeInfo() const;

    // -------------------------------------------------------------------
    // Component Configuration methods
    // -------------------------------------------------------------------

    [[nodiscard("config value should not be ignored!")]] std::optional<json>
    getValue(const std::string& key_path) const;

    /**
     * @brief 添加或更新一个配置项
     *
     * Add or update a configuration item.
     *
     * @param key_path 配置项的键路径，使用斜杠 / 进行分隔，如
     * "database/username"
     * @param value 配置项的值，使用 JSON 格式进行表示
     * @return bool 成功返回 true，失败返回 false
     */
    bool setValue(const std::string& key_path, const json& value);

    /**
     * @brief 判断一个配置项是否存在
     *
     * Determine if a configuration item exists.
     *
     * @param key_path 配置项的键路径，使用斜杠 / 进行分隔，如
     * "database/username"
     * @return bool 存在返回 true，不存在返回 false
     */
    [[nodiscard("status of the value should not be ignored")]] bool hasValue(
        const std::string& key_path) const;

    /**
     * @brief 从指定文件中加载JSON配置，并与原有配置进行合并
     *
     * Load JSON configuration from the specified file and merge with the
     * existing configuration.
     *
     * @param path 配置文件路径
     */
    bool loadFromFile(const fs::path& path);

    /**
     * @brief 将当前配置保存到指定文件
     *
     * Save the current configuration to the specified file.
     *
     * @param file_path 目标文件路径
     */
    bool saveToFile(const fs::path& file_path) const;

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

    template <typename Ret, typename... Args>
    void registerCommand(
        const std::string& name, const std::string& group,
        const std::string& description, std::function<Ret(Args...)> func,
        std::optional<std::function<bool()>> precondition = std::nullopt,
        std::optional<std::function<void()>> postcondition = std::nullopt) {
        m_CommandDispatcher->registerCommand(name, group, description, func,
                                             precondition, postcondition);
    }

    template <typename Callable>
    void registerCommand(const std::string& name, Callable&& func,
                         const std::string& group = "",
                         const std::string& description = "") {
        m_CommandDispatcher->registerCommand(name, func, group, description);
    }

    template <typename Ret, typename... Args>
    void registerCommand(const std::string& name, Ret (*func)(Args...),
                         const std::string& group = "",
                         const std::string& description = "") {
        m_CommandDispatcher->registerCommand(name, func, group, description);
    }

    template <typename Ret, typename Class, typename... Args>
    void registerCommand(const std::string& name, Ret (Class::*func)(Args...),
                         const PointerSentinel<Class>& instance,
                         const std::string& group = "",
                         const std::string& description = "")

    {
        m_CommandDispatcher->registerCommand(name, func, instance, group,
                                             description);
    }

    template <typename Ret, typename Class, typename... Args>
    void registerCommand(const std::string& name,
                         Ret (Class::*func)(Args...) const,
                         const PointerSentinel<Class>& instance,
                         const std::string& group = "",
                         const std::string& description = "") {
        m_CommandDispatcher->registerCommand(name, func, instance, group,
                                             description);
    }

    template <typename Ret, typename Class, typename... Args>
    void registerCommand(const std::string& name,
                         Ret (Class::*func)(Args...) noexcept,
                         const PointerSentinel<Class>& instance,
                         const std::string& group = "",
                         const std::string& description = "") {
        m_CommandDispatcher->registerCommand(name, func, instance, group,
                                             description);
    }

    template <typename Ret, typename Class, typename... Args>
    void registerCommand(const std::string& name, Ret (*func)(Args...),
                         const std::string& group = "",
                         const std::string& description = "") {
        m_CommandDispatcher->registerCommand(name, func, group, description);
    }

    void addAlias(const std::string& name, const std::string& alias);

    void addGroup(const std::string& name, const std::string& group);

    void setTimeout(const std::string& name, std::chrono::milliseconds timeout);

    template <typename... Args>
    std::any dispatch(const std::string& name, Args&&... args) {
        return m_CommandDispatcher->dispatch(name, std::forward<Args>(args)...);
    }

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
                           const PointerSentinel<Component>& component);

    void removeOtherComponent(const std::string& name);

    void clearOtherComponents();

private:
    std::string m_name;
    std::string m_configPath;
    std::string m_infoPath;
    Type_Info m_typeInfo;

    std::unique_ptr<CommandDispatcher>
        m_CommandDispatcher;  ///< The command dispatcher for managing commands.
    std::unique_ptr<VariableManager>
        m_VariableManager;  ///< The variable registry for managing variables.
    std::unique_ptr<ConfigManager> m_ConfigManager;

    std::unordered_map<std::string, PointerSentinel<Component>> m_OtherComponents;

    std::mutex m_mutex;
};

#endif
