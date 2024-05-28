#ifndef ATOM_COMPONENT_DISPATCH_INL
#define ATOM_COMPONENT_DISPATCH_INL

#include "dispatch.hpp"

#include "atom/error/exception.hpp"
#include "atom/function/abi.hpp"
#include "atom/function/func_traits.hpp"

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
void CommandDispatcher::def(
    const std::string& name, const std::string& group,
    const std::string& description, std::function<Ret(Args...)> func,
    std::optional<std::function<bool()>> precondition,
    std::optional<std::function<void()>> postcondition) {
    auto _func = atom::meta::ProxyFunction(std::move(func));
    auto info = _func.getFunctionInfo();
    if (info.argumentNames.size() != info.argumentTypes.size()) {
        THROW_DISPATCH_EXCEPTION(
            "Argument names and types must be the same size");
    }
    auto it = commands.find(name);
    if (it == commands.end()) {
        Command cmd{{std::move(_func)},
                    {},
                    {info.argumentNames},
                    {info.argumentTypes},
                    description,
                    {},
                    std::move(precondition),
                    std::move(postcondition)};
        commands[name] = std::move(cmd);
        groupMap[name] = group;
    } else {
        it->second.funcs.emplace_back(std::move(_func));
        it->second.arg_types.emplace_back(info.argumentTypes);
        it->second.arg_names.emplace_back(info.argumentNames);
    }
}

template <typename Ret, typename... Args>
void CommandDispatcher::def_t(
    const std::string& name, const std::string& group,
    const std::string& description, std::function<Ret(Args...)> func,
    std::optional<std::function<bool()>> precondition,
    std::optional<std::function<void()>> postcondition) {
    auto _func = atom::meta::TimerProxyFunction(std::move(func));
    auto it = commands.find(name);
    if (it == commands.end()) {
        Command cmd{{std::move(_func)},
                    {},
                    {},
                    {},
                    description,
                    {},
                    std::move(precondition),
                    std::move(postcondition)};
        commands[name] = std::move(cmd);
        groupMap[name] = group;
    } else {
        it->second.funcs.emplace_back(std::move(_func));
    }
    auto info = _func.getFunctionInfo();
    it->second.arg_names.emplace_back(info.argumentNames);
    it->second.arg_types.emplace_back(info.argumentTypes);
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
    if (cmd.precondition.has_value() && !cmd.precondition.value()()) {
        THROW_DISPATCH_EXCEPTION("Precondition failed for command: " + name);
    }

    auto cacheIt = cacheMap.find(name);
    if (cacheIt != cacheMap.end()) {
        return cacheIt->second;
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
        cacheMap[name] = result;
        if (cmd.postcondition.has_value())
            cmd.postcondition.value()();
        return result;
    } else {
        // Max: 如果只有一个重载函数，直接调用
        if (cmd.funcs.size() == 1) {
            try {
                auto result = cmd.funcs[0](args);
                cacheMap[name] = result;
                if (cmd.postcondition.has_value())
                    cmd.postcondition.value()();
                return result;
            } catch (const std::bad_any_cast&) {
                // 这里不需要重载函数，因此重新抛出异常
                THROW_DISPATCH_EXCEPTION(
                    "No matching overload found for command: " + name)
            }
        } else if (cmd.funcs.size() > 1) {
            // std::string func_hash;
            //  Max: 仅支持 std::vector<std::any>
            // if constexpr (std::is_same_v<ArgsType, std::vector<std::any>>) {
            // func_hash = atom::algorithm::computeHash(args);
            //  TODO
            for (const auto& func : cmd.funcs) {
                try {
                    // Max: 这里用函数的hash来匹配，是根据注册时计算得到
                    auto result = func(args);
                    cacheMap[name] = result;
                    if (cmd.postcondition.has_value())
                        cmd.postcondition.value()();
                    return result;
                } catch (const std::bad_any_cast&) {
                    // 参数类型不匹配, 尝试下一个重载函数
                }
                //}
                //}
                // THROW_INVALID_ARGUMENT("Variadic arguments are not support
                // yet");
            }
            // 这个其实不太可能发生，但还是要抛出异常
            THROW_INVALID_ARGUMENT("No matching overload found for command: " +
                                   name);
        }
        THROW_INVALID_ARGUMENT("No overload found for command: " + name);
    }
}
#endif
