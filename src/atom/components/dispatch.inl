#ifndef ATOM_COMPONENT_DISPATCH_INL
#define ATOM_COMPONENT_DISPATCH_INL

#include "dispatch.hpp"

#include "atom/error/exception.hpp"
#include "atom/function/abi.hpp"
#include "atom/function/func_traits.hpp"

template <typename Ret, typename... Args>
void CommandDispatcher::def(
    const std::string& name, const std::string& group,
    const std::string& description, std::function<Ret(Args...)> func,
    std::optional<std::function<bool()>> precondition,
    std::optional<std::function<void()>> postcondition) {
    auto it = commands.find(name);
    if (it == commands.end()) {
        Command cmd{{ProxyFunction(std::move(func))},
                    {},
                    description,
                    {},
                    std::move(precondition),
                    std::move(postcondition)};
        commands[name] = std::move(cmd);
        groupMap[name] = group;
    } else {
        it->second.funcs.emplace_back(ProxyFunction(std::move(func)));
    }
}

template <typename Callable>
void CommandDispatcher::def(const std::string& name,
                                        Callable&& func,
                                        const std::string& group,
                                        const std::string& description) {
    def(name, group, description,
                    std::function(std::forward<Callable>(func)));
}

template <typename Ret>
void def(const std::string& name, Ret (*func)(),
                     const std::string& group = "",
                     const std::string& description = "") {
    def(name, group, description, std::function<Ret()>(func));
}

template <typename... Args, typename Ret>
void CommandDispatcher::def(const std::string& name,
                                        Ret (*func)(Args...),
                                        const std::string& group,
                                        const std::string& description) {
    def(name, group, description,
                    std::function<Ret(Args...)>([func](Args... args) {
                        return func(std::forward<Args>(args)...);
                    }));
}

template <typename Ret, typename Class>
void CommandDispatcher::def(const std::string& name,
                                        Ret (Class::*func)(),
                                        std::shared_ptr<Class> instance,
                                        const std::string& group,
                                        const std::string& description) {
    def(name, group, description,
                    std::function<Ret()>([instance, func]() {
                        return std::invoke(func, instance.get());
                    }));
}

template <typename... Args, typename Ret, typename Class>
void CommandDispatcher::def(const std::string& name,
                                        Ret (Class::*func)(Args...),
                                        std::shared_ptr<Class> instance,
                                        const std::string& group,
                                        const std::string& description) {
    def(name, group, description,
                    std::function<Ret(Args...)>([instance, func](Args... args) {
                        return std::invoke(func, instance.get(),
                                           std::forward<Args>(args)...);
                    }));
}

template <typename... Args, typename Ret, typename Class>
void CommandDispatcher::def(const std::string& name,
                                        Ret (Class::*func)(Args...) const,
                                        std::shared_ptr<Class> instance,
                                        const std::string& group,
                                        const std::string& description) {
    def(name, group, description,
                    std::function<Ret(Args...)>([instance, func](Args... args) {
                        return std::invoke(func, instance.get(),
                                           std::forward<Args>(args)...);
                    }));
}

template <typename Ret, typename Class>
void def(const std::string& name, Ret (Class::*func)(),
                     const PointerSentinel<Class>& instance,
                     const std::string& group = "",
                     const std::string& description = "") {
    def(name, group, description,
                    std::function<Ret()>([instance, func]() {
                        return std::invoke(func, instance.get());
                    }));
}

template <typename... Args, typename Ret, typename Class>
void CommandDispatcher::def(const std::string& name,
                                        Ret (Class::*func)(Args...),
                                        const PointerSentinel<Class>& instance,
                                        const std::string& group,
                                        const std::string& description) {
    def(name, group, description,
                    std::function<Ret(Args...)>([instance, func](Args... args) {
                        return std::invoke(func, instance.get(),
                                           std::forward<Args>(args)...);
                    }));
}

template <typename... Args, typename Ret, typename Class>
void CommandDispatcher::def(const std::string& name,
                                        Ret (Class::*func)(Args...) const,
                                        const PointerSentinel<Class>& instance,
                                        const std::string& group,
                                        const std::string& description) {
    def(name, group, description,
                    std::function<Ret(Args...)>([instance, func](Args... args) {
                        return std::invoke(func, instance.get(),
                                           std::forward<Args>(args)...);
                    }));
}

template <typename... Args, typename Ret, typename Class>
void CommandDispatcher::def(const std::string& name,
                                        Ret (Class::*func)(Args...) noexcept,
                                        const PointerSentinel<Class>& instance,
                                        const std::string& group,
                                        const std::string& description) {
    def(name, group, description,
                    std::function<Ret(Args...)>([instance, func](Args... args) {
                        return std::invoke(func, instance.get(),
                                           std::forward<Args>(args)...);
                    }));
}

template <typename... Args, typename Ret, typename Class>
void CommandDispatcher::def(const std::string& name,
                                        Ret (*func)(Args...),
                                        const std::string& group,
                                        const std::string& description) {
    def(name, group, description,
                    std::function<Ret(Args...)>([func](Args... args) {
                        return func(std::forward<Args>(args)...);
                    }));
}

template <typename... Args>
std::any CommandDispatcher::dispatch(const std::string& name, Args&&... args) {
    auto it = commands.find(name);
    if (it == commands.end()) {
        for (const auto& cmd : commands) {
            for (const auto& alias : cmd.second.aliases) {
                if (alias == name) {
#if ENABLE_DEBUG
                    std::cout << "Command '" << name
                              << "' not found, did you mean '" << cmd.first
                              << "'?\n";
#endif
                    it = commands.find(cmd.first);
                }
            }
        }
        throw std::runtime_error("Unknown command: " + name);
    }

    const auto& cmd = it->second;
    if (cmd.precondition.has_value() && !cmd.precondition.value()()) {
        throw std::runtime_error("Precondition failed for command: " + name);
    }

    auto argsTuple = std::make_tuple(std::forward<Args>(args)...);
    std::vector<std::any> argsVec;
    argsVec.reserve(sizeof...(Args));
    std::apply(
        [&argsVec](auto&&... args) {
            ((argsVec.emplace_back(std::forward<decltype(args)>(args))), ...);
        },
        argsTuple);

    auto cacheIt = cacheMap.find(name);
    if (cacheIt != cacheMap.end()) {
        return cacheIt->second;
    }

    auto timeoutIt = timeoutMap.find(name);
    if (timeoutIt != timeoutMap.end()) {
        auto future = std::async(std::launch::async, [&]() {
            for (const auto& func : cmd.funcs) {
                try {
                    // Max: 匹配找到的第一个，但是理论上来说不会有重复
                    return func(argsVec);
                } catch (const std::bad_any_cast&) {
                    // 参数类型不匹配,尝试下一个重载函数
                }
            }
            return std::any{};
        });
        if (future.wait_for(timeoutIt->second) == std::future_status::timeout) {
            throw std::runtime_error("Command timed out: " + name);
        }
        auto result = future.get();
        cacheMap[name] = result;
        if (cmd.postcondition.has_value())
            cmd.postcondition.value()();
        return result;
    } else {
        for (const auto& func : cmd.funcs) {
            try {
                auto result = func(argsVec);
                cacheMap[name] = result;
                if (cmd.postcondition.has_value())
                    cmd.postcondition.value()();
                return result;
            } catch (const std::bad_any_cast&) {
                // 参数类型不匹配,尝试下一个重载函数
            }
        }
        throw std::runtime_error("No matching overload found for command: " +
                                 name);
    }
}

#endif