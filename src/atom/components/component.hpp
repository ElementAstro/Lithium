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

#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

#include "dispatch.hpp"
#include "error/exception.hpp"
#include "macro.hpp"
#include "module_macro.hpp"
#include "types.hpp"
#include "var.hpp"

#include "atom/function/constructor.hpp"
#include "atom/function/conversion.hpp"
#include "atom/function/type_caster.hpp"
#include "atom/function/type_info.hpp"
#include "atom/log/loguru.hpp"

class Component : public std::enable_shared_from_this<Component> {
public:
    /**
     * @brief Type definition for initialization function.
     */
    using InitFunc = std::function<void(Component&)>;

    /**
     * @brief Type definition for cleanup function.
     */
    using CleanupFunc = std::function<void()>;

    /**
     * @brief Constructs a new Component object.
     */
    explicit Component(std::string name);

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
    virtual auto initialize() -> bool;

    /**
     * @brief Destroys the plugin.
     *
     * @return True if the plugin was destroyed successfully, false otherwise.
     * @note This function is called by the server when the plugin is unloaded.
     * @note This function should be overridden by the plugin.
     * @note The plugin should not be used after this function is called.
     * @note This is for the plugin to release any resources it has allocated.
     */
    virtual auto destroy() -> bool;

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
    atom::meta::TypeInfo getTypeInfo() const;

    void setTypeInfo(atom::meta::TypeInfo typeInfo);

    // -------------------------------------------------------------------
    // Variable methods
    // -------------------------------------------------------------------

    template <typename T>
    void addVariable(const std::string& name, T initialValue,
                     const std::string& description = "",
                     const std::string& alias = "",
                     const std::string& group = "") {
        m_VariableManager_->addVariable(name, initialValue, description, alias,
                                        group);
    }

    template <typename T>
    void setRange(const std::string& name, T min, T max) {
        m_VariableManager_->setRange(name, min, max);
    }

    void setStringOptions(const std::string& name,
                          const std::vector<std::string>& options) {
        m_VariableManager_->setStringOptions(name, options);
    }

    /**
     * @brief Gets a variable by name.
     * @param name The name of the variable.
     * @return A shared pointer to the variable.
     */
    template <typename T>
    std::shared_ptr<Trackable<T>> getVariable(const std::string& name) {
        return m_VariableManager_->getVariable<T>(name);
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
        m_VariableManager_->setValue(name, newValue);
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
    void defM(const std::string& name, MemberType ClassType::*member_var,
              std::shared_ptr<ClassType> instance,
              const std::string& group = "",
              const std::string& description = "");

    template <typename MemberType, typename ClassType>
    void defM(const std::string& name, MemberType ClassType::*member_var,
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
    void defConstructor(const std::string& name, const std::string& group = "",
                        const std::string& description = "");

    template <typename Class>
    void defDefaultConstructor(const std::string& name,
                               const std::string& group = "",
                               const std::string& description = "");

    template <typename T>
    void defType(std::string_view name, const std::string& group = "",
                 const std::string& description = "");

    template <typename EnumType>
    void defEnum(const std::string& name,
                 const std::unordered_map<std::string, EnumType>& enumMap);

    template <typename SourceType, typename DestinationType>
    void defConversion(std::function<std::any(const std::any&)> func);

    template <typename Base, typename Derived>
    void defBaseClass();

    void defClassConversion(
        const std::shared_ptr<atom::meta::TypeConversionBase>& conversion);

    void addAlias(const std::string& name, const std::string& alias) const;

    void addGroup(const std::string& name, const std::string& group) const;

    void setTimeout(const std::string& name,
                    std::chrono::milliseconds timeout) const;

    template <typename... Args>
    auto dispatch(const std::string& name, Args&&... args) -> std::any {
        return m_CommandDispatcher_->dispatch(name,
                                              std::forward<Args>(args)...);
    }

    auto dispatch(const std::string& name,
                  const std::vector<std::any>& args) const -> std::any {
        return m_CommandDispatcher_->dispatch(name, args);
    }

    [[nodiscard]] auto has(const std::string& name) const -> bool;

    [[nodiscard]] auto hasType(std::string_view name) const -> bool;

    template <typename SourceType, typename DestinationType>
    [[nodiscard]] auto hasConversion() const -> bool;

    void removeCommand(const std::string& name) const;

    auto getCommandsInGroup(const std::string& group) const
        -> std::vector<std::string>;

    auto getCommandDescription(const std::string& name) const -> std::string;

#if ENABLE_FASTHASH
    emhash::HashSet<std::string> getCommandAliases(
        const std::string& name) const;
#else
    auto getCommandAliases(const std::string& name) const
        -> std::unordered_set<std::string>;
#endif

    auto getAllCommands() const -> std::vector<std::string>;

    auto getRegisteredTypes() const -> std::vector<std::string>;

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
    static auto getNeededComponents() -> std::vector<std::string>;

    void addOtherComponent(const std::string& name,
                           const std::weak_ptr<Component>& component);

    void removeOtherComponent(const std::string& name);

    void clearOtherComponents();

    auto getOtherComponent(const std::string& name) -> std::weak_ptr<Component>;

    auto runCommand(const std::string& name,
                    const std::vector<std::any>& args) -> std::any;

    InitFunc initFunc; /**< The initialization function for the component. */
    CleanupFunc cleanupFunc; /**< The cleanup function for the component. */

private:
    template <typename MemberType, typename ClassType, typename InstanceType>
    void defineAccessors(const std::string& name,
                         MemberType ClassType::*member_var,
                         InstanceType instance, const std::string& group = "",
                         const std::string& description = "");

    std::string m_name_;
    std::string m_doc_;
    std::string m_configPath_;
    std::string m_infoPath_;
    atom::meta::TypeInfo m_typeInfo_{atom::meta::userType<Component>()};
    std::unordered_map<std::string_view, atom::meta::TypeInfo> m_classes_;

    ///< managing commands.
    std::shared_ptr<VariableManager> m_VariableManager_{
        std::make_shared<VariableManager>()};  ///< The variable registry for
                                               ///< managing variables.

    std::unordered_map<std::string, std::weak_ptr<Component>>
        m_OtherComponents_;

    std::shared_ptr<atom::meta::TypeCaster> m_TypeCaster_{
        atom::meta::TypeCaster::createShared()};
    std::shared_ptr<atom::meta::TypeConversions> m_TypeConverter_{
        atom::meta::TypeConversions::createShared()};

    std::shared_ptr<CommandDispatcher> m_CommandDispatcher_{
        std::make_shared<CommandDispatcher>(
            m_TypeCaster_)};  ///< The command dispatcher for
};

ATOM_INLINE Component::Component(std::string name) : m_name_(std::move(name)) {}

ATOM_INLINE auto Component::getInstance() const
    -> std::weak_ptr<const Component> {
    return shared_from_this();
}

ATOM_INLINE auto Component::initialize() -> bool {
    LOG_F(INFO, "Initializing component: {}", m_name_);
    return true;
}

ATOM_INLINE auto Component::destroy() -> bool {
    LOG_F(INFO, "Destroying component: {}", m_name_);
    return true;
}

ATOM_INLINE auto Component::getName() const -> std::string { return m_name_; }

ATOM_INLINE auto Component::getTypeInfo() const -> atom::meta::TypeInfo {
    return m_typeInfo_;
}

ATOM_INLINE void Component::setTypeInfo(atom::meta::TypeInfo typeInfo) {
    m_typeInfo_ = typeInfo;
}

ATOM_INLINE void Component::addAlias(const std::string& name,
                                     const std::string& alias) const {
    m_CommandDispatcher_->addAlias(name, alias);
}

ATOM_INLINE void Component::addGroup(const std::string& name,
                                     const std::string& group) const {
    m_CommandDispatcher_->addGroup(name, group);
}

ATOM_INLINE void Component::setTimeout(
    const std::string& name, std::chrono::milliseconds timeout) const {
    m_CommandDispatcher_->setTimeout(name, timeout);
}

ATOM_INLINE void Component::removeCommand(const std::string& name) const {
    m_CommandDispatcher_->removeCommand(name);
}

ATOM_INLINE auto Component::getCommandsInGroup(const std::string& group) const
    -> std::vector<std::string> {
    return m_CommandDispatcher_->getCommandsInGroup(group);
}

ATOM_INLINE auto Component::getCommandDescription(const std::string& name) const
    -> std::string {
    return m_CommandDispatcher_->getCommandDescription(name);
}

#if ENABLE_FASTHASH
ATOM_INLINE emhash::HashSet<std::string> Component::getCommandAliases(
    const std::string& name) const
#else
ATOM_INLINE auto Component::getCommandAliases(const std::string& name) const
    -> std::unordered_set<std::string>
#endif
{
    return m_CommandDispatcher_->getCommandAliases(name);
}

ATOM_INLINE auto Component::getNeededComponents() -> std::vector<std::string> {
    return {};
}

ATOM_INLINE void Component::addOtherComponent(
    const std::string& name, const std::weak_ptr<Component>& component) {
    if (m_OtherComponents_.contains(name)) {
        THROW_OBJ_ALREADY_EXIST(name);
    }
    m_OtherComponents_[name] = component;
}

ATOM_INLINE void Component::removeOtherComponent(const std::string& name) {
    m_OtherComponents_.erase(name);
}

ATOM_INLINE void Component::clearOtherComponents() {
    m_OtherComponents_.clear();
}

ATOM_INLINE auto Component::getOtherComponent(const std::string& name)
    -> std::weak_ptr<Component> {
    if (m_OtherComponents_.contains(name)) {
        return m_OtherComponents_[name];
    }
    return {};
}

ATOM_INLINE bool Component::has(const std::string& name) const {
    return m_CommandDispatcher_->has(name);
}

ATOM_INLINE bool Component::hasType(std::string_view name) const {
    if (auto it = m_classes_.find(name); it != m_classes_.end()) {
        return true;
    }
    return false;
}

template <typename SourceType, typename DestinationType>
auto Component::hasConversion() const -> bool {
    if constexpr (std::is_same_v<SourceType, DestinationType>) {
        return true;
    }
    return m_TypeConverter_->canConvert(
        atom::meta::userType<SourceType>(),
        atom::meta::userType<DestinationType>());
}

ATOM_INLINE auto Component::getAllCommands() const -> std::vector<std::string> {
    if (m_CommandDispatcher_ == nullptr) {
        THROW_OBJ_UNINITIALIZED(
            "Component command dispatch is not initialized");
    }
    return m_CommandDispatcher_->getAllCommands();
}

ATOM_INLINE auto Component::getRegisteredTypes() const
    -> std::vector<std::string> {
    return m_TypeCaster_->getRegisteredTypes();
}

ATOM_INLINE auto Component::runCommand(
    const std::string& name, const std::vector<std::any>& args) -> std::any {
    auto cmd = getAllCommands();

    if (auto it = std::ranges::find(cmd, name); it != cmd.end()) {
        return m_CommandDispatcher_->dispatch(name, args);
    }
    for (const auto& [key, value] : m_OtherComponents_) {
        if (!value.expired() && value.lock()->has(name)) {
            return value.lock()->dispatch(name, args);
        }
        LOG_F(ERROR, "Component {} has expired", key);
        m_OtherComponents_.erase(key);
    }

    THROW_EXCEPTION("Component ", name, " not found");
}

ATOM_INLINE void Component::doc(const std::string& description) {
    m_doc_ = description;
}

template <typename T>
void Component::defType(std::string_view name,
                        [[maybe_unused]] const std::string& group,
                        [[maybe_unused]] const std::string& description) {
    m_classes_[name] = atom::meta::userType<T>();
    m_TypeCaster_->registerType<T>(std::string(name));
}

template <typename SourceType, typename DestinationType>
void Component::defConversion(std::function<std::any(const std::any&)> func) {
    static_assert(!std::is_same_v<SourceType, DestinationType>,
                  "SourceType and DestinationType must be not the same");
    m_TypeCaster_->registerConversion<SourceType, DestinationType>(func);
}

ATOM_INLINE void Component::defClassConversion(
    const std::shared_ptr<atom::meta::TypeConversionBase>& conversion) {
    m_TypeConverter_->addConversion(conversion);
}

template <typename Base, typename Derived>
void Component::defBaseClass() {
    static_assert(std::is_base_of_v<Base, Derived>,
                  "Derived must be derived from Base");
    m_TypeConverter_->addBaseClass<Base, Derived>();
}

ATOM_INLINE auto Component::hasVariable(const std::string& name) const -> bool {
    return m_VariableManager_->has(name);
}

ATOM_INLINE auto Component::getVariableDescription(
    const std::string& name) const -> std::string {
    return m_VariableManager_->getDescription(name);
}

ATOM_INLINE auto Component::getVariableAlias(const std::string& name) const
    -> std::string {
    return m_VariableManager_->getAlias(name);
}

ATOM_INLINE auto Component::getVariableGroup(const std::string& name) const
    -> std::string {
    return m_VariableManager_->getGroup(name);
}

template <typename Callable>
void Component::def(const std::string& name, Callable&& func,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher_->def(name, group, description,
                              std::function(std::forward<Callable>(func)));
}

template <typename Ret>
void Component::def(const std::string& name, Ret (*func)(),
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher_->def(name, group, description,
                              std::function<Ret()>(func));
}

template <typename... Args, typename Ret>
void Component::def(const std::string& name, Ret (*func)(Args...),
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher_->def(name, group, description,
                              std::function<Ret(Args...)>([func](Args... args) {
                                  return func(std::forward<Args>(args)...);
                              }));
}

template <typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*func)(),
                    std::shared_ptr<Class> instance, const std::string& group,
                    const std::string& description) {
    m_CommandDispatcher_->def(name, group, description,
                              std::function<Ret()>([instance, func]() {
                                  return std::invoke(func, instance.get());
                              }));
}

template <typename... Args, typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*func)(Args...),
                    std::shared_ptr<Class> instance, const std::string& group,
                    const std::string& description) {
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<Ret(Args...)>([instance, func](Args... args) {
            return std::invoke(func, instance.get(),
                               std::forward<Args>(args)...);
        }));
}

template <typename... Args, typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*func)(Args...) const,
                    std::shared_ptr<Class> instance, const std::string& group,
                    const std::string& description) {
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<Ret(Args...)>([instance, func](Args... args) {
            return std::invoke(func, instance.get(),
                               std::forward<Args>(args)...);
        }));
}

template <typename Class, typename Ret, typename... Args>
void Component::def(const std::string& name, Ret (Class::*func)(Args...),
                    const std::string& group, const std::string& description) {
    auto boundFunc = atom::meta::bindMemberFunction(func);
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<Ret(Class&, Args...)>(
            [boundFunc](Class& instance, Args... args) {
                return boundFunc(instance, std::forward<Args>(args)...);
            }));
}

template <typename Class, typename Ret, typename... Args>
void Component::def(const std::string& name, Ret (Class::*func)(Args...) const,
                    const std::string& group, const std::string& description) {
    auto boundFunc = atom::meta::bindMemberFunction(func);
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<Ret(Class&, Args...)>(
            [boundFunc](Class& instance, Args... args) -> Ret {
                return boundFunc(instance, std::forward<Args>(args)...);
            }));
}

template <typename Class, typename VarType>
void Component::def_v(const std::string& name, VarType Class::*var,
                      const std::string& group,
                      const std::string& description) {
    auto boundVar = atom::meta::bindMemberVariable(var);
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<VarType(Class&)>(
            [boundVar](Class& instance) { return boundVar(instance); }));
}

template <typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*func)(),
                    const PointerSentinel<Class>& instance,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher_->def(name, group, description,
                              std::function<Ret()>([instance, func]() {
                                  return std::invoke(func, instance.get());
                              }));
}

template <typename... Args, typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*func)(Args...),
                    const PointerSentinel<Class>& instance,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<Ret(Args...)>([instance, func](Args... args) {
            return std::invoke(func, instance.get(),
                               std::forward<Args>(args)...);
        }));
}

template <typename... Args, typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*func)(Args...) const,
                    const PointerSentinel<Class>& instance,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<Ret(Args...)>([instance, func](Args... args) {
            return std::invoke(func, instance.get(),
                               std::forward<Args>(args)...);
        }));
}

template <typename... Args, typename Ret, typename Class>
void Component::def(const std::string& name,
                    Ret (Class::*func)(Args...) noexcept,
                    const PointerSentinel<Class>& instance,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<Ret(Args...)>([instance, func](Args... args) {
            return std::invoke(func, instance.get(),
                               std::forward<Args>(args)...);
        }));
}

template <typename MemberType, typename ClassType>
void Component::def(const std::string& name, MemberType ClassType::*member_var,
                    std::shared_ptr<ClassType> instance,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher_->def(
        "get_" + name, group, "Get " + description,
        std::function<MemberType()>([instance, member_var]() {
            return atom::meta::bindMemberVariable(member_var)(*instance);
        }));
    m_CommandDispatcher_->def(
        "set_" + name, group, "Set " + description,
        std::function<void(MemberType)>(
            [instance, member_var](MemberType value) {
                atom::meta::bindMemberVariable(member_var)(*instance) = value;
            }));
}

template <typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*getter)() const,
                    void (Class::*setter)(Ret), std::shared_ptr<Class> instance,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher_->def("get_" + name, group, "Get " + description,
                              std::function<Ret()>([instance, getter]() {
                                  return std::invoke(getter, instance.get());
                              }));
    m_CommandDispatcher_->def(
        "set_" + name, group, "Set " + description,
        std::function<void(Ret)>([instance, setter](Ret value) {
            std::invoke(setter, instance.get(), value);
        }));
}

template <typename MemberType, typename ClassType>
void Component::def(const std::string& name, MemberType ClassType::*member_var,
                    const PointerSentinel<ClassType>& instance,
                    const std::string& group, const std::string& description) {
    auto callable = bind_member_variable(member_var);
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<MemberType&(ClassType&)>(
            [instance, callable]() { return callable(*instance.get()); }));
}

template <typename MemberType, typename ClassType>
void Component::def(const std::string& name,
                    const MemberType ClassType::*member_var,
                    std::shared_ptr<ClassType> instance,
                    const std::string& group, const std::string& description) {
    auto callable = bind_member_variable(member_var);
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<const MemberType&(ClassType&)>(
            [instance, callable]() { return callable(*instance); }));
}

template <typename MemberType, typename ClassType>
void Component::def(const std::string& name,
                    const MemberType ClassType::*member_var,
                    const PointerSentinel<ClassType>& instance,
                    const std::string& group, const std::string& description) {
    auto callable = bind_member_variable(member_var);
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<const MemberType&(ClassType&)>(
            [instance, callable]() { return callable(*instance.get()); }));
}

template <typename MemberType, typename ClassType>
void Component::def(const std::string& name, MemberType* member_var,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<MemberType&()>(
            [member_var]() -> MemberType& { return *member_var; }));
}

template <typename MemberType, typename ClassType>
void Component::def(const std::string& name, const MemberType* member_var,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<const MemberType&()>(
            [member_var]() -> const MemberType& { return *member_var; }));
}

template <typename Class, typename... Args>
void Component::defConstructor(const std::string& name,
                               const std::string& group,
                               const std::string& description) {
    m_CommandDispatcher_->def(name, group, description,
                              std::function<std::shared_ptr<Class>(Args...)>(
                                  atom::meta::constructor<Class, Args...>()));
}

template <typename EnumType>
void Component::defEnum(
    const std::string& name,
    const std::unordered_map<std::string, EnumType>& enumMap) {
    m_TypeCaster_->registerType<EnumType>(std::string(name));

    for (const auto& [key, value] : enumMap) {
        m_TypeCaster_->registerEnumValue<EnumType>(name, key, value);
    }

    defConversion<EnumType, std::string>(
        [this, name](const std::any& enumValue) -> std::any {
            const EnumType& value = std::any_cast<EnumType>(enumValue);
            return m_TypeCaster_->enumToString<EnumType>(value, name);
        });

    defConversion<std::string, EnumType>(
        [this, name](const std::any& strValue) -> std::any {
            const std::string& value = std::any_cast<std::string>(strValue);
            return m_TypeCaster_->stringToEnum<EnumType>(value, name);
        });
}

template <typename Class>
void Component::defDefaultConstructor(const std::string& name,
                                      const std::string& group,
                                      const std::string& description) {
    m_CommandDispatcher_->def(
        name, group, description,
        std::function<std::shared_ptr<Class>()>([]() -> std::shared_ptr<Class> {
            return std::make_shared<Class>();
        }));
}

template <typename MemberType, typename ClassType>
void Component::defM(const std::string& name, MemberType ClassType::*member_var,
                     std::shared_ptr<ClassType> instance,
                     const std::string& group, const std::string& description) {
    define_accessors(name, member_var, instance, group, description);
}

template <typename MemberType, typename ClassType>
void Component::defM(const std::string& name, MemberType ClassType::*member_var,
                     PointerSentinel<ClassType> instance,
                     const std::string& group, const std::string& description) {
    define_accessors(name, member_var, instance, group, description);
}

template <typename Class>
void Component::def(const std::string& name, const std::string& group,
                    const std::string& description) {
    auto constructor = atom::meta::defaultConstructor<Class>();
    def(name, constructor, group, description);
}

template <typename Class, typename... Args>
void Component::def(const std::string& name, const std::string& group,
                    const std::string& description) {
    auto constructor_ = atom::meta::constructor<Class, Args...>();
    def(name, constructor_, group, description);
}

template <typename MemberType, typename ClassType, typename InstanceType>
void Component::defineAccessors(const std::string& name,
                                MemberType ClassType::*member_var,
                                InstanceType instance, const std::string& group,
                                const std::string& description) {
    auto getter = [instance, member_var]() -> MemberType& {
        return instance->*member_var;
    };

    auto setter = [instance, member_var](const MemberType& value) {
        instance->*member_var = value;
    };

    m_CommandDispatcher_->def("get_" + name, group, description,
                              std::function<MemberType&()>(getter));
    m_CommandDispatcher_->def("set_" + name, group, description,
                              std::function<void(const MemberType&)>(setter));
}

#endif
