#ifndef ATOM_COMPONENT_DISPATCH_INL
#define ATOM_COMPONENT_DISPATCH_INL

#include "dispatch.hpp"
#include "macro.hpp"

#include "atom/error/exception.hpp"
#include "atom/function/abi.hpp"
#include "atom/function/func_traits.hpp"

#include "atom/utils/to_string.hpp"

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

template <typename Ret, typename... Args>
void CommandDispatcher::def(const std::string& name, const std::string& group,
                            const std::string& description,
                            std::function<Ret(Args...)> func,
                            std::optional<std::function<bool()>> precondition,
                            std::optional<std::function<void()>> postcondition,
                            std::vector<Arg> arg_info) {
    auto _func = atom::meta::ProxyFunction(std::move(func));
    auto info = _func.getFunctionInfo();
    auto it = commands.find(name);
    if (it == commands.end()) {
        Command cmd{{std::move(_func)},
                    {info.returnType},
                    {info.argumentTypes},
                    {info.hash},
                    description,
                    {},
                    std::move(precondition),
                    std::move(postcondition),
                    std::move(arg_info)};
        commands[name] = std::move(cmd);
        groupMap[name] = group;
    } else {
        it->second.funcs.emplace_back(std::move(_func));
        it->second.return_type.emplace_back(info.returnType);
        it->second.arg_types.emplace_back(info.argumentTypes);
        it->second.hash.emplace_back(info.hash);
        it->second.arg_info = std::move(arg_info);
    }
}

template <typename Ret, typename... Args>
void CommandDispatcher::def_t(
    const std::string& name, const std::string& group,
    const std::string& description, std::function<Ret(Args...)> func,
    std::optional<std::function<bool()>> precondition,
    std::optional<std::function<void()>> postcondition,
    std::vector<Arg> arg_info) {
    auto _func = atom::meta::TimerProxyFunction(std::move(func));
    auto info = _func.getFunctionInfo();
    auto it = commands.find(name);
    if (it == commands.end()) {
        Command cmd{{std::move(_func)},
                    {info.returnType},
                    {info.argumentTypes},
                    {info.hash},
                    description,
                    {},
                    std::move(precondition),
                    std::move(postcondition),
                    std::move(arg_info)};
        commands[name] = std::move(cmd);
        groupMap[name] = group;
    } else {
        it->second.funcs.emplace_back(std::move(_func));
        it->second.return_type.emplace_back(info.returnType);
        it->second.arg_types.emplace_back(info.argumentTypes);
        it->second.hash.emplace_back(info.hash);
        it->second.arg_info = std::move(arg_info);
    }
}

template <typename... Args>
std::any CommandDispatcher::dispatch(const std::string& name, Args&&... args) {
    auto argsTuple = std::make_tuple(std::forward<Args>(args)...);
    auto argsVec = convertToArgsVector(std::move(argsTuple));
    return dispatchHelper(name, argsVec);
}

template <typename... Args>
std::vector<std::any> CommandDispatcher::convertToArgsVector(
    std::tuple<Args...>&& tuple) {
    std::vector<std::any> argsVec;
    argsVec.reserve(sizeof...(Args));
    std::apply(
        [&argsVec](auto&&... args) {
            ((argsVec.emplace_back(std::forward<decltype(args)>(args))), ...);
        },
        std::move(tuple));
    return argsVec;
}

template <typename ArgsType>
std::any CommandDispatcher::dispatchHelper(const std::string& name,
                                           const ArgsType& args) {
    auto it = findCommand(name);
    if (it == commands.end()) {
        THROW_INVALID_ARGUMENT("Unknown command: " + name);
    }

    const auto& cmd = it->second;
    std::vector<std::any> fullArgs = completeArgs(cmd, args);

    checkPrecondition(cmd, name);

    auto result = executeCommand(cmd, name, fullArgs);

    if (cmd.postcondition) {
        cmd.postcondition.value()();
    }

    return result;
}

auto CommandDispatcher::findCommand(const std::string& name) {
    auto it = commands.find(name);
    if (it == commands.end()) {
        for (const auto& [cmdName, cmd] : commands) {
            if (std::find(cmd.aliases.begin(), cmd.aliases.end(), name) !=
                cmd.aliases.end()) {
#if ENABLE_DEBUG
                std::cout << "Command '" << name
                          << "' not found, did you mean '" << cmdName << "'?\n";
#endif
                return commands.find(cmdName);
            }
        }
    }
    return it;
}

template <typename ArgsType>
std::vector<std::any> CommandDispatcher::completeArgs(const Command& cmd,
                                                      const ArgsType& args) {
    std::vector<std::any> fullArgs(args.begin(), args.end());
    for (size_t i = args.size(); i < cmd.arg_info.size(); ++i) {
        if (cmd.arg_info[i].getDefaultValue()) {
            fullArgs.push_back(cmd.arg_info[i].getDefaultValue().value());
        } else {
            THROW_INVALID_ARGUMENT("Missing argument: " +
                                   cmd.arg_info[i].getName());
        }
    }
    return fullArgs;
}

void CommandDispatcher::checkPrecondition(const Command& cmd,
                                          const std::string& name) {
    if (cmd.precondition && !cmd.precondition.value()()) {
        THROW_DISPATCH_EXCEPTION("Precondition failed for command: " + name);
    }
}

std::any CommandDispatcher::executeCommand(const Command& cmd,
                                           const std::string& name,
                                           const std::vector<std::any>& args) {
    auto timeoutIt = timeoutMap.find(name);
    if (timeoutIt != timeoutMap.end()) {
        return executeWithTimeout(cmd, name, args, timeoutIt->second);
    } else {
        return executeWithoutTimeout(cmd, name, args);
    }
}

std::any CommandDispatcher::executeWithTimeout(
    const Command& cmd, const std::string& name,
    const std::vector<std::any>& args,
    const std::chrono::duration<double>& timeout) {
    auto future = std::async(std::launch::async,
                             [&]() { return executeFunctions(cmd, args); });

    if (future.wait_for(timeout) == std::future_status::timeout) {
        THROW_DISPATCH_TIMEOUT("Command timed out: " + name);
    }

    return future.get();
}

std::any CommandDispatcher::executeWithoutTimeout(
    const Command& cmd, const std::string& name,
    const std::vector<std::any>& args) {
    if (cmd.funcs.size() == 1) {
        try {
            return cmd.funcs[0](args);
        } catch (const std::bad_any_cast&) {
            THROW_DISPATCH_EXCEPTION("Bad command invoke: " + name);
        }
    }

    return executeFunctions(cmd, args);
}

std::any CommandDispatcher::executeFunctions(
    const Command& cmd, const std::vector<std::any>& args) {
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

std::string CommandDispatcher::computeFunctionHash(
    const std::vector<std::any>& args) {
    std::vector<std::string> argTypes;
    for (const auto& arg : args) {
        argTypes.emplace_back(
            atom::meta::DemangleHelper::Demangle(arg.type().name()));
    }
    return atom::utils::toString(atom::algorithm::computeHash(argTypes));
}

ATOM_INLINE bool CommandDispatcher::has(const std::string& name) const {
    if (commands.find(name) != commands.end())
        return true;
    for (const auto& command : commands) {
        if (command.second.aliases.find(name) != command.second.aliases.end())
            return true;
    }
    return false;
}

ATOM_INLINE void CommandDispatcher::addAlias(const std::string& name,
                                             const std::string& alias) {
    auto it = commands.find(name);
    if (it != commands.end()) {
        it->second.aliases.insert(alias);
        commands[alias] = it->second;
        groupMap[alias] = groupMap[name];
    }
}

ATOM_INLINE void CommandDispatcher::addGroup(const std::string& name,
                                             const std::string& group) {
    groupMap[name] = group;
}

ATOM_INLINE void CommandDispatcher::setTimeout(
    const std::string& name, std::chrono::milliseconds timeout) {
    timeoutMap[name] = timeout;
}

ATOM_INLINE void CommandDispatcher::removeCommand(const std::string& name) {
    commands.erase(name);
    groupMap.erase(name);
    timeoutMap.erase(name);
}

ATOM_INLINE std::vector<std::string> CommandDispatcher::getCommandsInGroup(
    const std::string& group) const {
    std::vector<std::string> result;
    for (const auto& pair : groupMap) {
        if (pair.second == group) {
            result.push_back(pair.first);
        }
    }
    return result;
}

ATOM_INLINE std::string CommandDispatcher::getCommandDescription(
    const std::string& name) const {
    auto it = commands.find(name);
    if (it != commands.end()) {
        return it->second.description;
    }
    return "";
}

ATOM_INLINE std::unordered_set<std::string>
CommandDispatcher::getCommandAliases(const std::string& name) const {
    auto it = commands.find(name);
    if (it != commands.end()) {
        return it->second.aliases;
    }
    return {};
}

ATOM_INLINE std::any CommandDispatcher::dispatch(
    const std::string& name, const std::vector<std::any>& args) {
    return dispatchHelper(name, args);
}

ATOM_INLINE std::any CommandDispatcher::dispatch(const std::string& name,
                                                 const FunctionParams& params) {
    return dispatchHelper(name, params.to_vector());
}

ATOM_INLINE std::vector<std::string> CommandDispatcher::getAllCommands() const {
    std::vector<std::string> result;
    for (const auto& pair : commands) {
        result.push_back(pair.first);
    }
    // Max: Add aliases to the result vector
    for (const auto& command : commands) {
        for (const auto& alias : command.second.aliases) {
            result.push_back(alias);
        }
    }
    auto it = std::unique(result.begin(), result.end());
    result.erase(it, result.end());
    return result;
}

#endif
