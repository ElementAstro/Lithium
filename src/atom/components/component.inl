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
    auto callable = bind_member_variable(member_var);
    m_CommandDispatcher->def(
        name, group, description,
        std::function<MemberType&(ClassType&)>(
            [instance, callable]() { return callable(*instance); }));
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

template <typename MemberType, typename ClassType>
void Component::def_m(const std::string& name, MemberType ClassType::*member_var,
                    std::shared_ptr<ClassType> instance,
                    const std::string& group, const std::string& description) {
    define_accessors(name, member_var, instance, group, description);
}

template <typename MemberType, typename ClassType>
void Component::def_m(const std::string& name, MemberType ClassType::*member_var,
                    PointerSentinel<ClassType> instance,
                    const std::string& group, const std::string& description) {
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
