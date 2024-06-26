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
#include <vector>

#include "dispatch.hpp"
#include "macro.hpp"
#include "module_macro.hpp"
#include "registry.hpp"
#include "types.hpp"
#include "var.hpp"

#include "atom/function/constructor.hpp"
#include "atom/function/conversion.hpp"
#include "atom/function/type_caster.hpp"
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
    virtual ~Component() = default;

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
    atom::meta::Type_Info getTypeInfo() const;

    void setTypeInfo(atom::meta::Type_Info typeInfo);

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
                          const std::vector<std::string>& options) {
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

    [[nodiscard]] bool hasVariable(const std::string& name) const;

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

    std::vector<std::string> getVariableNames() const;

    std::string getVariableDescription(const std::string& name) const;

    std::string getVariableAlias(const std::string& name) const;

    std::string getVariableGroup(const std::string& name) const;

    // -------------------------------------------------------------------
    // Function methods
    // -------------------------------------------------------------------

    void doc(const std::string& description);

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

    template <typename Class, typename Ret, typename... Args>
    void def(const std::string& name, Ret (Class::*func)(Args...),
             const std::string& group = "",
             const std::string& description = "");

    template <typename Class, typename Ret, typename... Args>
    void def(const std::string& name, Ret (Class::*func)(Args...) const,
             const std::string& group = "",
             const std::string& description = "");

    template <typename Class, typename VarType>
    void def_v(const std::string& name, VarType Class::*var,
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

    // Register a member variable using a raw pointer sentinel
    template <typename MemberType, typename Class>
    void def(const std::string& name, MemberType Class::*var,
             const PointerSentinel<Class>& instance,
             const std::string& group = "",
             const std::string& description = "");

    // Register a member variable using a shared pointer
    template <typename MemberType, typename Class>
    void def(const std::string& name, MemberType Class::*var,
             std::shared_ptr<Class> instance, const std::string& group = "",
             const std::string& description = "");

    template <typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*getter)() const,
             void (Class::*setter)(Ret), std::shared_ptr<Class> instance,
             const std::string& group = "",
             const std::string& description = "");

    // Register a static member variable
    template <typename MemberType, typename Class>
    void def(const std::string& name, MemberType* var,
             const std::string& group = "",
             const std::string& description = "");

    // Register a const & static member variable
    template <typename MemberType, typename Class>
    void def(const std::string& name, const MemberType* var,
             const std::string& group = "",
             const std::string& description = "");

    // Register a const member variable
    template <typename MemberType, typename Class>
    void def(const std::string& name, const MemberType Class::*var,
             const PointerSentinel<Class>& instance,
             const std::string& group = "",
             const std::string& description = "");

    template <typename MemberType, typename Class>
    void def(const std::string& name, const MemberType Class::*var,
             std::shared_ptr<Class> instance, const std::string& group = "",
             const std::string& description = "");

    template <typename MemberType, typename ClassType>
    void def_m(const std::string& name, MemberType ClassType::*member_var,
               std::shared_ptr<ClassType> instance,
               const std::string& group = "",
               const std::string& description = "");

    template <typename MemberType, typename ClassType>
    void def_m(const std::string& name, MemberType ClassType::*member_var,
               PointerSentinel<ClassType> instance,
               const std::string& group = "",
               const std::string& description = "");

    template <typename Class>
    void def(const std::string& name, const std::string& group = "",
             const std::string& description = "");

    template <typename Class, typename... Args>
    void def(const std::string& name, const std::string& group = "",
             const std::string& description = "");

    template <typename Class, typename... Args>
    void def_constructor(const std::string& name, const std::string& group = "",
                         const std::string& description = "");

    template <typename Class>
    void def_default_constructor(const std::string& name,
                                 const std::string& group = "",
                                 const std::string& description = "");

    template <typename T>
    void def_type(std::string_view name, const atom::meta::Type_Info& ti,
                  const std::string& group = "",
                  const std::string& description = "");

    template <typename SourceType, typename DestinationType>
    void def_conversion(std::function<std::any(const std::any&)> func);

    template <typename Base, typename Derived>
    void def_base_class();

    void def_class_conversion(
        const std::shared_ptr<atom::meta::Type_Conversion_Base>& conversion);

    void addAlias(const std::string& name, const std::string& alias) const;

    void addGroup(const std::string& name, const std::string& group) const;

    void setTimeout(const std::string& name, std::chrono::milliseconds timeout) const;

    template <typename... Args>
    std::any dispatch(const std::string& name, Args&&... args) {
        return m_CommandDispatcher->dispatch(name, std::forward<Args>(args)...);
    }

    std::any dispatch(const std::string& name,
                      const std::vector<std::any>& args) const {
        return m_CommandDispatcher->dispatch(name, args);
    }

    [[nodiscard]] bool has(const std::string& name) const;

    [[nodiscard]] bool has_type(std::string_view name) const;

    template <typename SourceType, typename DestinationType>
    [[nodiscard]] bool has_conversion() const;

    void removeCommand(const std::string& name) const;

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

    std::vector<std::string> getRegisteredTypes() const;

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

    std::any runCommand(const std::string& name,
                        const std::vector<std::any>& args);

private:
    template <typename MemberType, typename ClassType, typename InstanceType>
    void define_accessors(const std::string& name,
                          MemberType ClassType::*member_var,
                          InstanceType instance, const std::string& group = "",
                          const std::string& description = "");

    std::string m_name;
    std::string m_doc;
    std::string m_configPath;
    std::string m_infoPath;
    atom::meta::Type_Info m_typeInfo{atom::meta::user_type<Component>()};
    std::unordered_map<std::string_view, atom::meta::Type_Info> m_classes;

    std::shared_ptr<CommandDispatcher> m_CommandDispatcher{
        std::make_shared<CommandDispatcher>()};  ///< The command dispatcher for
                                                 ///< managing commands.
    std::shared_ptr<VariableManager> m_VariableManager{
        std::make_shared<VariableManager>()};  ///< The variable registry for
                                               ///< managing variables.

    std::unordered_map<std::string, std::weak_ptr<Component>> m_OtherComponents;

    std::shared_ptr<atom::meta::TypeCaster> m_TypeCaster{
        atom::meta::TypeCaster::createShared()};
    std::shared_ptr<atom::meta::TypeConversions> m_TypeConverter{
        atom::meta::TypeConversions::createShared()};
};

#include "component.inl"

#endif
