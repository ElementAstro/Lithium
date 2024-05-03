#ifndef ATOM_COMMAND_DISPATCH_HPP
#define ATOM_COMMAND_DISPATCH_HPP

#include <any>
#include <chrono>
#include <concepts>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#if ENABLE_FASTHASH
#include "emhash/hash_set8.hpp"
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#include <unordered_set>
#endif
#include <variant>
#include <vector>

#include "atom/type/noncopyable.hpp"
#include "atom/type/pointer.hpp"

#include "atom/function/proxy.hpp"

class CommandDispatcher {
public:
    template <typename Ret, typename... Args>
    void def(
        const std::string& name, const std::string& group,
        const std::string& description, std::function<Ret(Args...)> func,
        std::optional<std::function<bool()>> precondition = std::nullopt,
        std::optional<std::function<void()>> postcondition = std::nullopt);

    template <typename Ret>
    void def(const std::string& name, Ret (*func)(),
                         const std::string& group = "",
                         const std::string& description = "");

    template <typename... Args, typename Ret>
    void def(const std::string& name, Ret (*func)(Args...),
                         const std::string& group = "",
                         const std::string& description = "");

    template <typename Callable>
    void def(const std::string& name, Callable&& func,
                         const std::string& group = "",
                         const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(),
                         const PointerSentinel<Class>& instance,
                         const std::string& group = "",
                         const std::string& description = "");

    template <typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(),
                         std::shared_ptr<Class> instance,
                         const std::string& group = "",
                         const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(Args...),
                         std::shared_ptr<Class> instance,
                         const std::string& group = "",
                         const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name,
                         Ret (Class::*func)(Args...) const,
                         std::shared_ptr<Class> instance,
                         const std::string& group = "",
                         const std::string& description = "");

    template <typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(),
                         const PointerSentinel<Class>& instance,
                         const std::string& group = "",
                         const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name, Ret (Class::*func)(Args...),
                         const PointerSentinel<Class>& instance,
                         const std::string& group = "",
                         const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name,
                         Ret (Class::*func)(Args...) const,
                         const PointerSentinel<Class>& instance,
                         const std::string& group = "",
                         const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name,
                         Ret (Class::*func)(Args...) noexcept,
                         const PointerSentinel<Class>& instance,
                         const std::string& group = "",
                         const std::string& description = "");

    template <typename... Args, typename Ret, typename Class>
    void def(const std::string& name, Ret (*func)(Args...),
                         const std::string& group = "",
                         const std::string& description = "");

    void addAlias(const std::string& name, const std::string& alias);

    void addGroup(const std::string& name, const std::string& group);

    void setTimeout(const std::string& name, std::chrono::milliseconds timeout);

    template <typename... Args>
    std::any dispatch(const std::string& name, Args&&... args);

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

private:
    struct Command {
        std::vector<std::function<std::any(const std::vector<std::any>&)>>
            funcs;
        std::vector<std::string> funcs_info;
        std::string description;
#if ENABLE_FASTHASH
        emhash::HashSet<std::string> aliases;
#else
        std::unordered_set<std::string> aliases;
#endif
        std::optional<std::function<bool()>> precondition;
        std::optional<std::function<void()>> postcondition;
    };

#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, Command> commands;
    emhash8::HashMap<std::string, std::string> groupMap;
    emhash8::HashMap<std::string, std::chrono::milliseconds> timeoutMap;
    emhash8::HashMap<std::string, std::any> cacheMap;
#else
    std::unordered_map<std::string, Command> commands;
    std::unordered_map<std::string, std::string> groupMap;
    std::unordered_map<std::string, std::chrono::milliseconds> timeoutMap;
    std::unordered_map<std::string, std::any> cacheMap;
#endif
};

#include "dispatch.inl"

#endif
