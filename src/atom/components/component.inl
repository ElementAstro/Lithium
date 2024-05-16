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
void Component::def(const std::string& name, Ret (Class::*func)(Args...) noexcept,
                    const PointerSentinel<Class>& instance,
                    const std::string& group, const std::string& description) {
    m_CommandDispatcher->def(
        name, group, description,
        std::function<Ret(Args...)>([instance, func](Args... args) {
            return std::invoke(func, instance.get(),
                               std::forward<Args>(args)...);
        }));
}

#endif