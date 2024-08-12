#ifndef ATOM_COMMAND_DISPATCH_HPP
#define ATOM_COMMAND_DISPATCH_HPP

#include <any>
#include <chrono>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>
#include "function/type_caster.hpp"

#if ENABLE_FASTHASH
#include "emhash/hash_set8.hpp"
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#include <unordered_set>
#endif

#include "atom/type/noncopyable.hpp"
#include "atom/type/pointer.hpp"

#include "atom/function/proxy.hpp"
#include "atom/function/type_info.hpp"
#include "macro.hpp"

#include "atom/error/exception.hpp"
#include "atom/function/abi.hpp"
#include "atom/function/func_traits.hpp"

#include "atom/utils/to_string.hpp"

class Arg {
public:
    explicit Arg(std::string name) : name_(std::move(name)) {}
    Arg(std::string name, std::any default_value)
        : name_(std::move(name)), default_value_(default_value) {}

    [[nodiscard]] auto getName() const -> const std::string& { return name_; }
    [[nodiscard]] auto getDefaultValue() const
        -> const std::optional<std::any>& {
        return default_value_;
    }

private:
    std::string name_;
    std::optional<std::any> default_value_;
};

class DispatchException : public atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_DISPATCH_EXCEPTION(...)                                       \
    throw DispatchException(ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, \
                            __VA_ARGS__);

class DispatchTimeout : public atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_DISPATCH_TIMEOUT(...)                                       \
    throw DispatchTimeout(ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, \
                          __VA_ARGS__);

class CommandDispatcher {
public:
    explicit CommandDispatcher(std::weak_ptr<atom::meta::TypeCaster> typeCaster)
        : typeCaster_(std::move(typeCaster)) {}

    template <typename Ret, typename... Args>
    void def(const std::string& name, const std::string& group,
             const std::string& description, std::function<Ret(Args...)> func,
             std::optional<std::function<bool()>> precondition = std::nullopt,
             std::optional<std::function<void()>> postcondition = std::nullopt,
             std::vector<Arg> arg_info = {});

    template <typename Ret, typename... Args>
    void defT(const std::string& name, const std::string& group,
              const std::string& description, std::function<Ret(Args...)> func,
              std::optional<std::function<bool()>> precondition = std::nullopt,
              std::optional<std::function<void()>> postcondition = std::nullopt,
              std::vector<Arg> arg_info = {});

    [[nodiscard]] auto has(const std::string& name) const -> bool;

    void addAlias(const std::string& name, const std::string& alias);

    void addGroup(const std::string& name, const std::string& group);

    void setTimeout(const std::string& name, std::chrono::milliseconds timeout);

    template <typename... Args>
    auto dispatch(const std::string& name, Args&&... args) -> std::any;

    auto dispatch(const std::string& name,
                  const std::vector<std::any>& args) -> std::any;

    auto dispatch(const std::string& name,
                  const FunctionParams& params) -> std::any;

    void removeCommand(const std::string& name);

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

private:
    struct Command;

    template <typename ArgsType>
    auto dispatchHelper(const std::string& name,
                        const ArgsType& args) -> std::any;

    template <typename... Args>
    auto convertToArgsVector(std::tuple<Args...>&& tuple)
        -> std::vector<std::any>;

    auto findCommand(const std::string& name);

    template <typename ArgsType>
    auto completeArgs(const Command& cmd,
                      const ArgsType& args) -> std::vector<std::any>;

    void checkPrecondition(const Command& cmd, const std::string& name);

    auto executeCommand(const Command& cmd, const std::string& name,
                        const std::vector<std::any>& args) -> std::any;

    auto executeWithTimeout(const Command& cmd, const std::string& name,
                            const std::vector<std::any>& args,
                            const std::chrono::duration<double>& timeout)
        -> std::any;

    auto executeWithoutTimeout(const Command& cmd, const std::string& name,
                               const std::vector<std::any>& args) -> std::any;

    auto executeFunctions(const Command& cmd,
                          const std::vector<std::any>& args) -> std::any;

    auto computeFunctionHash(const std::vector<std::any>& args) -> std::string;

    struct Command {
        std::vector<std::function<std::any(const std::vector<std::any>&)>>
            funcs;
        std::vector<std::string> returnType;
        std::vector<std::vector<std::string>> argTypes;
        std::vector<std::string> hash;
        std::string description;
#if ENABLE_FASTHASH
        emhash::HashSet<std::string> aliases;
#else
        std::unordered_set<std::string> aliases;
#endif
        std::optional<std::function<bool()>> precondition;
        std::optional<std::function<void()>> postcondition;
        std::vector<Arg> argInfo;
    } ATOM_ALIGNAS(128);

#if ENABLE_FASTHASH
    emhash8::HashMap<std::string, Command> commands;
    emhash8::HashMap<std::string, std::string> groupMap;
    emhash8::HashMap<std::string, std::chrono::milliseconds> timeoutMap;
#else
    std::unordered_map<std::string, Command> commands_;
    std::unordered_map<std::string, std::string> groupMap_;
    std::unordered_map<std::string, std::chrono::milliseconds> timeoutMap_;
#endif

    std::weak_ptr<atom::meta::TypeCaster> typeCaster_;
};

template <typename Ret, typename... Args>
void CommandDispatcher::def(const std::string& name, const std::string& group,
                            const std::string& description,
                            std::function<Ret(Args...)> func,
                            std::optional<std::function<bool()>> precondition,
                            std::optional<std::function<void()>> postcondition,
                            std::vector<Arg> arg_info) {
    auto _func = atom::meta::ProxyFunction(std::move(func));
    auto info = _func.getFunctionInfo();
    auto it = commands_.find(name);
    if (it == commands_.end()) {
        Command cmd{{std::move(_func)},
                    {info.returnType},
                    {info.argumentTypes},
                    {info.hash},
                    description,
                    {},
                    std::move(precondition),
                    std::move(postcondition),
                    std::move(arg_info)};
        commands_[name] = std::move(cmd);
        groupMap_[name] = group;
    } else {
        it->second.funcs.emplace_back(std::move(_func));
        it->second.returnType.emplace_back(info.returnType);
        it->second.argTypes.emplace_back(info.argumentTypes);
        it->second.hash.emplace_back(info.hash);
        it->second.argInfo = std::move(arg_info);
    }
}

template <typename Ret, typename... Args>
void CommandDispatcher::defT(const std::string& name, const std::string& group,
                             const std::string& description,
                             std::function<Ret(Args...)> func,
                             std::optional<std::function<bool()>> precondition,
                             std::optional<std::function<void()>> postcondition,
                             std::vector<Arg> arg_info) {
    auto _func = atom::meta::TimerProxyFunction(std::move(func));
    std::function<std::any(const std::vector<std::any>&)> wrappedFunc =
        [_func](const std::vector<std::any>& args) mutable -> std::any {
        std::chrono::milliseconds defaultTimeout(1000);
        return _func(args, defaultTimeout);
    };

    auto info = _func.getFunctionInfo();
    auto it = commands_.find(name);
    if (it == commands_.end()) {
        Command cmd{{std::move(wrappedFunc)},
                    {info.returnType},
                    {info.argumentTypes},
                    {info.hash},
                    description,
                    {},
                    std::move(precondition),
                    std::move(postcondition),
                    std::move(arg_info)};
        commands_[name] = std::move(cmd);
        groupMap_[name] = group;
    } else {
        it->second.funcs.emplace_back(std::move(wrappedFunc));
        it->second.returnType.emplace_back(info.returnType);
        it->second.argTypes.emplace_back(info.argumentTypes);
        it->second.hash.emplace_back(info.hash);
        it->second.argInfo = std::move(arg_info);
    }
}

template <typename... Args>
auto CommandDispatcher::dispatch(const std::string& name,
                                 Args&&... args) -> std::any {
    auto argsTuple = std::make_tuple(std::forward<Args>(args)...);
    auto argsVec = convertToArgsVector(std::move(argsTuple));
    return dispatchHelper(name, argsVec);
}

template <typename... Args>
auto CommandDispatcher::convertToArgsVector(std::tuple<Args...>&& tuple)
    -> std::vector<std::any> {
    std::vector<std::any> argsVec;
    argsVec.reserve(sizeof...(Args));
    std::apply(
        [&argsVec](auto&&... args) {
            ((argsVec.emplace_back(std::forward<decltype(args)>(args))), ...);
        },
        std::move(tuple));
    return argsVec;
}

ATOM_INLINE auto CommandDispatcher::findCommand(const std::string& name) {
    auto it = commands_.find(name);
    if (it == commands_.end()) {
        for (const auto& [cmdName, cmd] : commands_) {
            if (std::find(cmd.aliases.begin(), cmd.aliases.end(), name) !=
                cmd.aliases.end()) {
#if ENABLE_DEBUG
                std::cout << "Command '" << name
                          << "' not found, did you mean '" << cmdName << "'?\n";
#endif
                return commands_.find(cmdName);
            }
        }
    }
    return it;
}

template <typename ArgsType>
auto CommandDispatcher::dispatchHelper(const std::string& name,
                                       const ArgsType& args) -> std::any {
    auto it = findCommand(name);
    if (it == commands_.end()) {
        THROW_INVALID_ARGUMENT("Unknown command: " + name);
    }

    const auto& cmd = it->second;
    std::vector<std::any> fullArgs;
    // if constexpr (std::is_same_v<ArgsType, std::vector<std::any>>) {
    //     fullArgs = args;
    // } else {
    fullArgs = completeArgs(cmd, args);
    //}

    if constexpr (std::is_same_v<ArgsType, std::vector<std::any>>) {
        auto it1 = args.begin();
        auto it2 = cmd.argTypes.begin();
        // Max: 这里需要自动处理类型差异
        for (; it1 != args.end() && it2 != cmd.argTypes.end(); ++it1, ++it2) {
        }
    }

    checkPrecondition(cmd, name);

    auto result = executeCommand(cmd, name, fullArgs);

    if (cmd.postcondition) {
        cmd.postcondition.value()();
    }

    return result;
}

template <typename ArgsType>
auto CommandDispatcher::completeArgs(const Command& cmd, const ArgsType& args)
    -> std::vector<std::any> {
    std::vector<std::any> fullArgs(args.begin(), args.end());
    for (size_t i = args.size(); i < cmd.argInfo.size(); ++i) {
        if (cmd.argInfo[i].getDefaultValue()) {
            fullArgs.push_back(cmd.argInfo[i].getDefaultValue().value());
        } else {
            THROW_INVALID_ARGUMENT("Missing argument: " +
                                   cmd.argInfo[i].getName());
        }
    }
    return fullArgs;
}

ATOM_INLINE void CommandDispatcher::checkPrecondition(const Command& cmd,
                                                      const std::string& name) {
    if (cmd.precondition && !cmd.precondition.value()()) {
        THROW_DISPATCH_EXCEPTION("Precondition failed for command: " + name);
    }
}

ATOM_INLINE auto CommandDispatcher::executeCommand(
    const Command& cmd, const std::string& name,
    const std::vector<std::any>& args) -> std::any {
    auto timeoutIt = timeoutMap_.find(name);
    if (timeoutIt != timeoutMap_.end()) {
        return executeWithTimeout(cmd, name, args, timeoutIt->second);
    }
    return executeWithoutTimeout(cmd, name, args);
}

ATOM_INLINE auto CommandDispatcher::executeWithTimeout(
    const Command& cmd, const std::string& name,
    const std::vector<std::any>& args,
    const std::chrono::duration<double>& timeout) -> std::any {
    auto future = std::async(std::launch::async,
                             [&]() { return executeFunctions(cmd, args); });

    if (future.wait_for(timeout) == std::future_status::timeout) {
        THROW_DISPATCH_TIMEOUT("Command timed out: " + name);
    }

    return future.get();
}

ATOM_INLINE auto CommandDispatcher::executeWithoutTimeout(
    const Command& cmd, [[maybe_unused]] const std::string& name,
    const std::vector<std::any>& args) -> std::any {
    if (!args.empty()) {
        if (args.size() == 1 &&
            args[0].type() == typeid(std::vector<std::any>)) {
            return executeFunctions(
                cmd, std::any_cast<std::vector<std::any>>(args[0]));
        }
    }

    return executeFunctions(cmd, args);
}

ATOM_INLINE auto CommandDispatcher::executeFunctions(
    const Command& cmd, const std::vector<std::any>& args) -> std::any {
    // TODO: FIX ME - Overload resolution
    if (cmd.funcs.size() == 1) {
        return cmd.funcs[0](args);
    }

    std::string funcHash = computeFunctionHash(args);
    for (size_t i = 0; i < cmd.funcs.size(); ++i) {
        if (cmd.hash[i] == funcHash) {
            try {
                return cmd.funcs[i](args);
            } catch (const std::bad_any_cast&) {
                THROW_DISPATCH_EXCEPTION("Failed to call function with hash " +
                                         funcHash);
            }
        }
    }

    THROW_INVALID_ARGUMENT("No matching overload found");
}

ATOM_INLINE auto CommandDispatcher::computeFunctionHash(
    const std::vector<std::any>& args) -> std::string {
    std::vector<std::string> argTypes;
    argTypes.reserve(args.size());
    for (const auto& arg : args) {
        argTypes.emplace_back(
            atom::meta::DemangleHelper::demangle(arg.type().name()));
    }
    return atom::utils::toString(atom::algorithm::computeHash(argTypes));
}

ATOM_INLINE auto CommandDispatcher::has(const std::string& name) const -> bool {
    if (commands_.find(name) != commands_.end()) {
        return true;
    }
    for (const auto& command : commands_) {
        if (command.second.aliases.find(name) != command.second.aliases.end()) {
            return true;
        }
    }
    return false;
}

ATOM_INLINE void CommandDispatcher::addAlias(const std::string& name,
                                             const std::string& alias) {
    auto it = commands_.find(name);
    if (it != commands_.end()) {
        it->second.aliases.insert(alias);
        commands_[alias] = it->second;
        groupMap_[alias] = groupMap_[name];
    }
}

ATOM_INLINE void CommandDispatcher::addGroup(const std::string& name,
                                             const std::string& group) {
    groupMap_[name] = group;
}

ATOM_INLINE void CommandDispatcher::setTimeout(
    const std::string& name, std::chrono::milliseconds timeout) {
    timeoutMap_[name] = timeout;
}

ATOM_INLINE void CommandDispatcher::removeCommand(const std::string& name) {
    commands_.erase(name);
    groupMap_.erase(name);
    timeoutMap_.erase(name);
}

ATOM_INLINE auto CommandDispatcher::getCommandsInGroup(
    const std::string& group) const -> std::vector<std::string> {
    std::vector<std::string> result;
    for (const auto& pair : groupMap_) {
        if (pair.second == group) {
            result.push_back(pair.first);
        }
    }
    return result;
}

ATOM_INLINE auto CommandDispatcher::getCommandDescription(
    const std::string& name) const -> std::string {
    auto it = commands_.find(name);
    if (it != commands_.end()) {
        return it->second.description;
    }
    return "";
}

ATOM_INLINE auto CommandDispatcher::getCommandAliases(
    const std::string& name) const -> std::unordered_set<std::string> {
    auto it = commands_.find(name);
    if (it != commands_.end()) {
        return it->second.aliases;
    }
    return {};
}

ATOM_INLINE auto CommandDispatcher::dispatch(
    const std::string& name, const std::vector<std::any>& args) -> std::any {
    return dispatchHelper(name, args);
}

ATOM_INLINE auto CommandDispatcher::dispatch(
    const std::string& name, const FunctionParams& params) -> std::any {
    return dispatchHelper(name, params.toVector());
}

ATOM_INLINE auto CommandDispatcher::getAllCommands() const
    -> std::vector<std::string> {
    std::vector<std::string> result;
    result.reserve(commands_.size());
    for (const auto& pair : commands_) {
        result.push_back(pair.first);
    }
    // Max: Add aliases to the result vector
    for (const auto& command : commands_) {
        for (const auto& alias : command.second.aliases) {
            result.push_back(alias);
        }
    }
    auto it = std::unique(result.begin(), result.end());
    result.erase(it, result.end());
    return result;
}

#endif
