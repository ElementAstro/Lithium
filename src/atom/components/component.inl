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

ATOM_INLINE Component::Component(const std::string& name) : m_name(name) {}

ATOM_INLINE std::weak_ptr<const Component> Component::getInstance() const {
    return shared_from_this();
}

ATOM_INLINE bool Component::initialize() {
    LOG_F(INFO, "Initializing component: {}", m_name);
    return true;
}

ATOM_INLINE bool Component::destroy() {
    LOG_F(INFO, "Destroying component: {}", m_name);
    return true;
}

ATOM_INLINE std::string Component::getName() const { return m_name; }

ATOM_INLINE atom::meta::TypeInfo Component::getTypeInfo() const {
    return m_typeInfo;
}

ATOM_INLINE void Component::setTypeInfo(atom::meta::TypeInfo typeInfo) {
    m_typeInfo = typeInfo;
}

ATOM_INLINE void Component::addAlias(const std::string& name,
                                     const std::string& alias) const {
    m_CommandDispatcher->addAlias(name, alias);
}

ATOM_INLINE void Component::addGroup(const std::string& name,
                                     const std::string& group) const {
    m_CommandDispatcher->addGroup(name, group);
}

ATOM_INLINE void Component::setTimeout(
    const std::string& name, std::chrono::milliseconds timeout) const {
    m_CommandDispatcher->setTimeout(name, timeout);
}

ATOM_INLINE void Component::removeCommand(const std::string& name) const {
    m_CommandDispatcher->removeCommand(name);
}

ATOM_INLINE std::vector<std::string> Component::getCommandsInGroup(
    const std::string& group) const {
    return m_CommandDispatcher->getCommandsInGroup(group);
}

ATOM_INLINE std::string Component::getCommandDescription(
    const std::string& name) const {
    return m_CommandDispatcher->getCommandDescription(name);
}

#if ENABLE_FASTHASH
ATOM_INLINE emhash::HashSet<std::string> Component::getCommandAliases(
    const std::string& name) const
#else
ATOM_INLINE std::unordered_set<std::string> Component::getCommandAliases(
    const std::string& name) const
#endif
{
    return m_CommandDispatcher->getCommandAliases(name);
}

ATOM_INLINE std::vector<std::string> Component::getNeededComponents() const {
    return {};
}

ATOM_INLINE void Component::addOtherComponent(
    const std::string& name, const std::weak_ptr<Component>& component) {
    if (m_OtherComponents.contains(name)) {
        THROW_OBJ_ALREADY_EXIST(name);
    }
    m_OtherComponents[name] = component;
}

ATOM_INLINE void Component::removeOtherComponent(const std::string& name) {
    m_OtherComponents.erase(name);
}

ATOM_INLINE void Component::clearOtherComponents() {
    m_OtherComponents.clear();
}

ATOM_INLINE std::weak_ptr<Component> Component::getOtherComponent(
    const std::string& name) {
    if (m_OtherComponents.contains(name)) {
        return m_OtherComponents[name];
    }
    return {};
}

ATOM_INLINE bool Component::has(const std::string& name) const {
    return m_CommandDispatcher->has(name);
}

ATOM_INLINE bool Component::has_type(std::string_view name) const {
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

ATOM_INLINE std::vector<std::string> Component::getAllCommands() const {
    return m_CommandDispatcher->getAllCommands();
}

ATOM_INLINE std::vector<std::string> Component::getRegisteredTypes() const {
    return m_TypeCaster->get_registered_types();
}

ATOM_INLINE std::any Component::runCommand(const std::string& name,
                                           const std::vector<std::any>& args) {
    auto _cmd = getAllCommands();

    if (auto it = std::ranges::find(_cmd, name); it != _cmd.end()) {
        return m_CommandDispatcher->dispatch(name, args);
    } else {
        for (const auto& [key, value] : m_OtherComponents) {
            if (!value.expired() && value.lock()->has(name)) {
                return value.lock()->dispatch(name, args);
            } else {
                LOG_F(ERROR, "Component {} has expired", key);
                m_OtherComponents.erase(key);
            }
        }
    }
    THROW_EXCEPTION("Component ", name, " not found");
}

ATOM_INLINE void Component::doc(const std::string& description) {
    m_doc = description;
}

template <typename T>
void Component::def_type(std::string_view name, const atom::meta::TypeInfo& ti,
                         [[maybe_unused]] const std::string& group,
                         [[maybe_unused]] const std::string& description) {
    m_classes[name] = ti;
    m_TypeCaster->register_type<T>(std::string(name));
}

template <typename SourceType, typename DestinationType>
void Component::def_conversion(std::function<std::any(const std::any&)> func) {
    static_assert(!std::is_same_v<SourceType, DestinationType>,
                  "SourceType and DestinationType must be not the same");
    m_TypeCaster->register_conversion<SourceType, DestinationType>(func);
}

ATOM_INLINE void Component::def_class_conversion(
    const std::shared_ptr<atom::meta::Type_Conversion_Base>& conversion) {
    m_TypeConverter->add_conversion(conversion);
}

template <typename Base, typename Derived>
void Component::def_base_class() {
    static_assert(std::is_base_of_v<Base, Derived>,
                  "Derived must be derived from Base");
    m_TypeConverter->add_base_class<Base, Derived>();
}

ATOM_INLINE bool Component::hasVariable(const std::string& name) const {
    return m_VariableManager->has(name);
}

ATOM_INLINE std::string Component::getVariableDescription(
    const std::string& name) const {
    return m_VariableManager->getDescription(name);
}

ATOM_INLINE std::string Component::getVariableAlias(
    const std::string& name) const {
    return m_VariableManager->getAlias(name);
}

ATOM_INLINE std::string Component::getVariableGroup(
    const std::string& name) const {
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
    auto boundFunc = atom::meta::bind_member_function(func);
    m_CommandDispatcher->def(name, group, description,
                             std::function<Ret(Class&, Args...)>(
                                 [boundFunc](Class& instance, Args... args) {
                                     return boundFunc(
                                         instance, std::forward<Args>(args)...);
                                 }));
}

template <typename Class, typename Ret, typename... Args>
void Component::def(const std::string& name, Ret (Class::*func)(Args...) const,
                    const std::string& group, const std::string& description) {
    auto boundFunc = atom::meta::bind_member_function(func);
    m_CommandDispatcher->def(
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
    auto boundVar = atom::meta::bind_member_variable(var);
    m_CommandDispatcher->def(
        name, group, description,
        std::function<VarType(Class&)>(
            [boundVar](Class& instance) { return boundVar(instance); }));
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
void Component::defineAccessors(const std::string& name,
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
