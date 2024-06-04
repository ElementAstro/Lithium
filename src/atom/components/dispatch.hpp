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
#include "atom/function/type_info.hpp"

class Arg {
public:
    Arg(const std::string& name) : name(name) {}
    Arg(const std::string& name, std::any default_value)
        : name(name), default_value(default_value) {}

    const std::string& getName() const { return name; }
    const std::optional<std::any>& getDefaultValue() const {
        return default_value;
    }

private:
    std::string name;
    std::optional<std::any> default_value;
};

class CommandDispatcher {
public:
    template <typename Ret, typename... Args>
    void def(const std::string& name, const std::string& group,
             const std::string& description, std::function<Ret(Args...)> func,
             std::optional<std::function<bool()>> precondition = std::nullopt,
             std::optional<std::function<void()>> postcondition = std::nullopt,
             std::vector<Arg> arg_info = {});

    template <typename Ret, typename... Args>
    void def_t(
        const std::string& name, const std::string& group,
        const std::string& description, std::function<Ret(Args...)> func,
        std::optional<std::function<bool()>> precondition = std::nullopt,
        std::optional<std::function<void()>> postcondition = std::nullopt,
        std::vector<Arg> arg_info = {});

    [[nodiscard]] bool has(const std::string& name) const;

    void addAlias(const std::string& name, const std::string& alias);

    void addGroup(const std::string& name, const std::string& group);

    void setTimeout(const std::string& name, std::chrono::milliseconds timeout);

    template <typename... Args>
    std::any dispatch(const std::string& name, Args&&... args);

    std::any dispatch(const std::string& name,
                      const std::vector<std::any>& args);

    std::any dispatch(const std::string& name, const FunctionParams& params);

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

private:
    template <typename ArgsType>
    std::any dispatchHelper(const std::string& name, const ArgsType& args);

    template <typename... Args>
    std::vector<std::any> convertToArgsVector(std::tuple<Args...>&& tuple);

private:
    struct Command {
        std::vector<std::function<std::any(const std::vector<std::any>&)>>
            funcs;
        std::vector<std::string> return_type;
        std::vector<std::vector<std::string>> arg_types;
        std::vector<std::string> hash;
        std::string description;
#if ENABLE_FASTHASH
        emhash::HashSet<std::string> aliases;
#else
        std::unordered_set<std::string> aliases;
#endif
        std::optional<std::function<bool()>> precondition;
        std::optional<std::function<void()>> postcondition;
        std::vector<Arg> arg_info;
    };

#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, Command> commands;
    emhash8::HashMap<std::string, std::string> groupMap;
    emhash8::HashMap<std::string, std::chrono::milliseconds> timeoutMap;
#else
    std::unordered_map<std::string, Command> commands;
    std::unordered_map<std::string, std::string> groupMap;
    std::unordered_map<std::string, std::chrono::milliseconds> timeoutMap;
#endif
};

#include "dispatch.inl"

#endif
