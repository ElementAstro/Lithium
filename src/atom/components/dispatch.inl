#ifndef ATOM_COMPONENT_DISPATCH_INL
#define ATOM_COMPONENT_DISPATCH_INL

#include "dispatch.hpp"

#include "atom/error/exception.hpp"
#include "atom/function/abi.hpp"
#include "atom/function/func_traits.hpp"

#include "atom/utils/to_string.hpp"

class DispatchException : public atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_DISPATCH_EXCEPTION(...) \
    throw DispatchException(__FILE__, __LINE__, __func__, __VA_ARGS__);

class DispatchTimeout : public atom::error::Exception {
public:
    using atom::error::Exception::Exception;
};

#define THROW_DISPATCH_TIMEOUT(...) \
    throw DispatchTimeout(__FILE__, __LINE__, __func__, __VA_ARGS__);

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
                    break;
                }
            }
            if (it != commands.end())
                break;
        }
        if (it == commands.end()) {
            THROW_INVALID_ARGUMENT("Unknown command: " + name);
        }
    }

    const auto& cmd = it->second;

    if (args.size() < cmd.arg_info.size()) {
        std::vector<std::any> full_args = args;
        for (size_t i = args.size(); i < cmd.arg_info.size(); ++i) {
            if (cmd.arg_info[i].getDefaultValue().has_value()) {
                full_args.push_back(cmd.arg_info[i].getDefaultValue().value());
            } else {
                THROW_INVALID_ARGUMENT("Missing argument: " +
                                       cmd.arg_info[i].getName());
            }
        }
        return dispatchHelper(name, full_args);
    }

    if (cmd.precondition.has_value() && !cmd.precondition.value()()) {
        THROW_DISPATCH_EXCEPTION("Precondition failed for command: " + name);
    }

    auto timeoutIt = timeoutMap.find(name);
    // Max: 超时函数的需求没有这么迫切，具体应该会有统一的线程池，现在仅作保留
    if (timeoutIt != timeoutMap.end()) {
        auto future = std::async(std::launch::async, [&]() {
            if (cmd.funcs.size() == 1) {
                return cmd.funcs[0](args);
            }
            for (const auto& func : cmd.funcs) {
                try {
                    return func(args);
                } catch (const std::bad_any_cast&) {
                    // 参数类型不匹配, 尝试下一个重载函数
                }
            }
            return std::any{};
        });
        if (future.wait_for(timeoutIt->second) == std::future_status::timeout) {
            THROW_DISPATCH_TIMEOUT("Command timed out: " + name);
        }
        auto result = future.get();
        if (cmd.postcondition.has_value())
            cmd.postcondition.value()();
        return result;
    } else {
        // Max: 如果只有一个重载函数，直接调用
        if (cmd.funcs.size() == 1) {
            try {
                auto result = cmd.funcs[0](args);
                if (cmd.postcondition.has_value())
                    cmd.postcondition.value()();
                return result;
            } catch (const std::bad_any_cast&) {
                // 这里不需要重载函数，因此重新抛出异常
                THROW_DISPATCH_EXCEPTION("Bad command invoke: " + name);
            }
        } else if (cmd.funcs.size() > 1) {
            if constexpr (std::is_same_v<ArgsType, std::vector<std::any>>) {
                std::string func_hash;
                std::vector<std::string> arg_types;
                for (const auto& arg : args) {
                    arg_types.emplace_back(atom::meta::DemangleHelper::Demangle(
                        arg.type().name()));
                }
                func_hash = atom::algorithm::computeHash(arg_types);
                int i = 0;
                for (const auto& func : cmd.funcs) {
                    if (cmd.hash[i] == func_hash) {
                        try {
                            // Max: 这里用函数的hash来匹配，是根据注册时计算得到
                            auto result = func(args);
                            if (cmd.postcondition.has_value())
                                cmd.postcondition.value()();
                            return result;
                        } catch (const std::bad_any_cast&) {
                            // Max: 这里应该是不会出现异常的
                            THROW_DISPATCH_EXCEPTION(
                                "Failed to call function ", name,
                                "with function hash ", func_hash);
                        }
                    }
                }
            }

            THROW_INVALID_ARGUMENT("No matching overload found for command: " +
                                   name);
        }
        THROW_INVALID_ARGUMENT("No overload found for command: " + name);
    }
}

inline bool CommandDispatcher::has(const std::string& name) const {
    if (commands.find(name) != commands.end())
        return true;
    for (const auto& command : commands) {
        if (command.second.aliases.find(name) != command.second.aliases.end())
            return true;
    }
    return false;
}

inline void CommandDispatcher::addAlias(const std::string& name,
                                        const std::string& alias) {
    auto it = commands.find(name);
    if (it != commands.end()) {
        it->second.aliases.insert(alias);
        commands[alias] = it->second;
        groupMap[alias] = groupMap[name];
    }
}

inline void CommandDispatcher::addGroup(const std::string& name,
                                        const std::string& group) {
    groupMap[name] = group;
}

inline void CommandDispatcher::setTimeout(const std::string& name,
                                          std::chrono::milliseconds timeout) {
    timeoutMap[name] = timeout;
}

inline void CommandDispatcher::removeCommand(const std::string& name) {
    commands.erase(name);
    groupMap.erase(name);
    timeoutMap.erase(name);
}

inline std::vector<std::string> CommandDispatcher::getCommandsInGroup(
    const std::string& group) const {
    std::vector<std::string> result;
    for (const auto& pair : groupMap) {
        if (pair.second == group) {
            result.push_back(pair.first);
        }
    }
    return result;
}

inline std::string CommandDispatcher::getCommandDescription(
    const std::string& name) const {
    auto it = commands.find(name);
    if (it != commands.end()) {
        return it->second.description;
    }
    return "";
}

inline std::unordered_set<std::string> CommandDispatcher::getCommandAliases(
    const std::string& name) const {
    auto it = commands.find(name);
    if (it != commands.end()) {
        return it->second.aliases;
    }
    return {};
}

inline std::any CommandDispatcher::dispatch(const std::string& name,
                                            const std::vector<std::any>& args) {
    return dispatchHelper(name, args);
}

inline std::any CommandDispatcher::dispatch(const std::string& name,
                                            const FunctionParams& params) {
    return dispatchHelper(name, params.to_vector());
}

inline std::vector<std::string> CommandDispatcher::getAllCommands() const {
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
