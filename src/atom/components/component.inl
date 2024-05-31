/*
 * component.inl
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-12-26

Description: Basic Component Definition

**************************************************/

#ifndef ATOM_COMPONENT_COMPONENT_INL
#define ATOM_COMPONENT_COMPONENT_INL

#include "component.hpp"

#include "atom/log/loguru.hpp"

inline Component::Component(const std::string& name)
    : m_name(name),
      m_CommandDispatcher(std::make_unique<CommandDispatcher>()),
      m_VariableManager(std::make_unique<VariableManager>()),
      m_typeInfo(atom::meta::user_type<Component>()),
      m_TypeCaster(atom::meta::TypeCaster::createShared()),
      m_TypeConverter(atom::meta::TypeConversions::createShared()) {
    // Empty
}

inline Component::~Component() {
    // Empty
}

inline std::weak_ptr<const Component> Component::getInstance() const {
    return shared_from_this();
}

inline bool Component::initialize() {
    LOG_F(INFO, "Initializing component: {}", m_name);
    return true;
}

inline bool Component::destroy() {
    LOG_F(INFO, "Destroying component: {}", m_name);
    return true;
}

inline std::string Component::getName() const { return m_name; }

inline atom::meta::Type_Info Component::getTypeInfo() const {
    return m_typeInfo;
}

inline void Component::setTypeInfo(atom::meta::Type_Info typeInfo) {
    m_typeInfo = typeInfo;
}

inline void Component::addAlias(const std::string& name,
                                const std::string& alias) {
    m_CommandDispatcher->addAlias(name, alias);
}

inline void Component::addGroup(const std::string& name,
                                const std::string& group) {
    m_CommandDispatcher->addGroup(name, group);
}

inline void Component::setTimeout(const std::string& name,
                                  std::chrono::milliseconds timeout) {
    m_CommandDispatcher->setTimeout(name, timeout);
}

inline void Component::removeCommand(const std::string& name) {
    m_CommandDispatcher->removeCommand(name);
}

inline std::vector<std::string> Component::getCommandsInGroup(
    const std::string& group) const {
    return m_CommandDispatcher->getCommandsInGroup(group);
}

inline std::string Component::getCommandDescription(
    const std::string& name) const {
    return m_CommandDispatcher->getCommandDescription(name);
}

#if ENABLE_FASTHASH
inline emhash::HashSet<std::string> Component::getCommandAliases(
    const std::string& name) const
#else
inline std::unordered_set<std::string> Component::getCommandAliases(
    const std::string& name) const
#endif
{
    return m_CommandDispatcher->getCommandAliases(name);
}

inline std::vector<std::string> Component::getNeededComponents() const {
    return {};
}

inline void Component::addOtherComponent(
    const std::string& name, const std::weak_ptr<Component>& component) {
    if (m_OtherComponents.contains(name)) {
        THROW_EXCEPTION(
#if __cplusplus >= 202002L
            std::format("Other component with name {} already exists",
#else
            fmt::format("Other component with name {} already exists",
#endif
                        name));
    }
    m_OtherComponents[name] = std::move(component);
}

inline void Component::removeOtherComponent(const std::string& name) {
    m_OtherComponents.erase(name);
}

inline void Component::clearOtherComponents() { m_OtherComponents.clear(); }

inline std::weak_ptr<Component> Component::getOtherComponent(
    const std::string& name) {
    if (m_OtherComponents.contains(name)) {
        return m_OtherComponents[name];
    }
    return {};
}

inline bool Component::has(const std::string& name) const {
    return m_CommandDispatcher->has(name);
}

inline bool Component::has_type(std::string_view name) const {
    if (auto it = m_classes.find(name); it != m_classes.end()) {
        return true;
    }
    return false;
}

template <typename SourceType, typename DestinationType>
bool Component::has_conversion() const {
    if constexpr (std::is_same_v<SourceType, DestinationType>) {
        return true;
    }
    return m_TypeConverter->can_convert(
        atom::meta::user_type<SourceType>(),
        atom::meta::user_type<DestinationType>());
}

inline std::vector<std::string> Component::getAllCommands() const {
    return m_CommandDispatcher->getAllCommands();
}

std::vector<std::string> Component::getRegisteredTypes() const {
    return m_TypeCaster->get_registered_types();
}

inline std::any Component::runCommand(const std::string& name,
                                      const std::vector<std::any>& args) {
    auto _cmd = getAllCommands();
    auto it = std::find(_cmd.begin(), _cmd.end(), name);

    if (it != _cmd.end()) {
        return m_CommandDispatcher->dispatch(name, args);
    } else {
        for (auto& [key, value] : m_OtherComponents) {
            if (!value.expired()) {
                if (value.lock()->has(name)) {
                    return value.lock()->dispatch(name, args);
                }
            } else {
                LOG_F(ERROR, "Component {} has expired", key);
                m_OtherComponents.erase(key);
            }
        }
    }
    THROW_EXCEPTION(
#if __cplusplus >= 202002L
        std::format("Command with name {} not found",
#else
        fmt::format("Command with name {} not found",
#endif
                    name));
}

template <typename T>
void Component::def_type(std::string_view name, const atom::meta::Type_Info& ti,
                         const std::string& group,
                         const std::string& description) {
    m_classes[name] = ti;
    m_TypeCaster->register_type<T>(std::string(name));
}

template <typename SourceType, typename DestinationType>
void Component::def_conversion(std::function<std::any(const std::any&)> func) {
    static_assert(!std::is_same_v<SourceType, DestinationType>,
                  "SourceType and DestinationType must be not the same");
    m_TypeCaster->register_conversion<SourceType, DestinationType>(func);
}

inline void Component::def_class_conversion(
    const std::shared_ptr<atom::meta::Type_Conversion_Base>& conversion) {
    m_TypeConverter->add_conversion(conversion);
}

template <typename Base, typename Derived>
void Component::def_base_class() {
    static_assert(std::is_base_of_v<Base, Derived>,
                  "Derived must be derived from Base");
    m_TypeConverter->add_base_class<Base, Derived>();
}

inline std::string Component::getVariableDescription(
    const std::string& name) const {
    return m_VariableManager->getDescription(name);
}

inline std::string Component::getVariableAlias(const std::string& name) const {
    return m_VariableManager->getAlias(name);
}

inline std::string Component::getVariableGroup(const std::string& name) const {
    return m_VariableManager->getGroup(name);
}

template <typename Callable>
void Component::def(const std::string& name, Callable&& func,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher->def(name, group, description,
                             std::function(std::forward<Callable>(func)));
}

template <typename Ret>
void Component::def(const std::string& name, Ret (*func)(),
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher->def(name, group, description,
                             std::function<Ret()>(func));
}

template <typename... Args, typename Ret>
void Component::def(const std::string& name, Ret (*func)(Args...),
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher->def(name, group, description,
                             std::function<Ret(Args...)>([func](Args... args) {
                                 return func(std::forward<Args>(args)...);
                             }));
}

template <typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*func)(),
                    std::shared_ptr<Class> instance, const std::string& group,
                    const std::string& description) {
    m_CommandDispatcher->def(name, group, description,
                             std::function<Ret()>([instance, func]() {
                                 return std::invoke(func, instance.get());
                             }));
}

template <typename... Args, typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*func)(Args...),
                    std::shared_ptr<Class> instance, const std::string& group,
                    const std::string& description) {
    m_CommandDispatcher->def(
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
    m_CommandDispatcher->def(
        name, group, description,
        std::function<Ret(Args...)>([instance, func](Args... args) {
            return std::invoke(func, instance.get(),
                               std::forward<Args>(args)...);
        }));
}

template <typename Class, typename Ret, typename... Args>
void Component::def(const std::string& name, Ret (Class::*func)(Args...),
                    const std::string& group, const std::string& description) {
    auto bound_func = atom::meta::bind_member_function(func);
    m_CommandDispatcher->def(name, group, description,
                             std::function<Ret(Class&, Args...)>(
                                 [bound_func](Class& instance, Args... args) {
                                     return bound_func(
                                         instance, std::forward<Args>(args)...);
                                 }));
}

template <typename Class, typename Ret, typename... Args>
void Component::def(const std::string& name, Ret (Class::*func)(Args...) const,
                    const std::string& group, const std::string& description) {
    auto bound_func = atom::meta::bind_member_function(func);
    m_CommandDispatcher->def(
        name, group, description,
        std::function<Ret(const Class&, Args...)>(
            [bound_func](const Class& instance, Args... args) {
                return bound_func(instance, std::forward<Args>(args)...);
            }));
}

template <typename Class, typename VarType>
void Component::def_v(const std::string& name, VarType Class::*var,
                      const std::string& group,
                      const std::string& description) {
    auto bound_var = atom::meta::bind_member_variable(var);
    m_CommandDispatcher->def(
        name, group, description,
        std::function<VarType(Class&)>(
            [bound_var](Class& instance) { return bound_var(instance); }));
}

template <typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*func)(),
                    const PointerSentinel<Class>& instance,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher->def(name, group, description,
                             std::function<Ret()>([instance, func]() {
                                 return std::invoke(func, instance.get());
                             }));
}

template <typename... Args, typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*func)(Args...),
                    const PointerSentinel<Class>& instance,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher->def(
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
    m_CommandDispatcher->def(
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
    m_CommandDispatcher->def(
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
    m_CommandDispatcher->def(
        "get_" + name, group, "Get " + description,
        std::function<MemberType()>([instance, member_var]() {
            return atom::meta::bind_member_variable(member_var)(*instance);
        }));
    m_CommandDispatcher->def(
        "set_" + name, group, "Set " + description,
        std::function<void(MemberType)>(
            [instance, member_var](MemberType value) {
                atom::meta::bind_member_variable(member_var)(*instance) = value;
            }));
}

template <typename Ret, typename Class>
void Component::def(const std::string& name, Ret (Class::*getter)() const,
                    void (Class::*setter)(Ret), std::shared_ptr<Class> instance,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher->def("get_" + name, group, "Get " + description,
                             std::function<Ret()>([instance, getter]() {
                                 return std::invoke(getter, instance.get());
                             }));
    m_CommandDispatcher->def(
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
    m_CommandDispatcher->def(
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
    m_CommandDispatcher->def(
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
    m_CommandDispatcher->def(
        name, group, description,
        std::function<const MemberType&(ClassType&)>(
            [instance, callable]() { return callable(*instance.get()); }));
}

template <typename MemberType, typename ClassType>
void Component::def(const std::string& name, MemberType* member_var,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher->def(
        name, group, description,
        std::function<MemberType&()>(
            [member_var]() -> MemberType& { return *member_var; }));
}

template <typename MemberType, typename ClassType>
void Component::def(const std::string& name, const MemberType* member_var,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher->def(
        name, group, description,
        std::function<const MemberType&()>(
            [member_var]() -> const MemberType& { return *member_var; }));
}

template <typename Class, typename... Args>
void Component::def_constructor(const std::string& name,
                                const std::string& group,
                                const std::string& description) {
    m_CommandDispatcher->def(name, group, description,
                             std::function<std::shared_ptr<Class>(Args...)>(
                                 atom::meta::constructor<Class, Args...>()));
}

template <typename Class>
void Component::def_default_constructor(const std::string& name,
                                        const std::string& group,
                                        const std::string& description) {
    m_CommandDispatcher->def(
        name, group, description,
        std::function<std::shared_ptr<Class>()>([]() -> std::shared_ptr<Class> {
            return std::make_shared<Class>();
        }));
}

template <typename MemberType, typename ClassType>
void Component::def_m(const std::string& name,
                      MemberType ClassType::*member_var,
                      std::shared_ptr<ClassType> instance,
                      const std::string& group,
                      const std::string& description) {
    define_accessors(name, member_var, instance, group, description);
}

template <typename MemberType, typename ClassType>
void Component::def_m(const std::string& name,
                      MemberType ClassType::*member_var,
                      PointerSentinel<ClassType> instance,
                      const std::string& group,
                      const std::string& description) {
    define_accessors(name, member_var, instance, group, description);
}

template <typename Class>
void Component::def(const std::string& name, const std::string& group,
                    const std::string& description) {
    auto constructor = atom::meta::default_constructor<Class>();
    def(name, constructor, group, description);
}

template <typename Class, typename... Args>
void Component::def(const std::string& name, const std::string& group,
                    const std::string& description) {
    auto constructor_ = atom::meta::constructor<Class, Args...>();
    def(name, constructor_, group, description);
}

template <typename MemberType, typename ClassType, typename InstanceType>
void Component::define_accessors(const std::string& name,
                                 MemberType ClassType::*member_var,
                                 InstanceType instance,
                                 const std::string& group,
                                 const std::string& description) {
    auto getter = [instance, member_var]() -> MemberType& {
        return instance->*member_var;
    };

    auto setter = [instance, member_var](const MemberType& value) {
        instance->*member_var = value;
    };

    m_CommandDispatcher->def("get_" + name, group, description,
                             std::function<MemberType&()>(getter));
    m_CommandDispatcher->def("set_" + name, group, description,
                             std::function<void(const MemberType&)>(setter));
}

#endif
