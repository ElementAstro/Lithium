#include "dispatch.hpp"

#include "atom/log/loguru.hpp"
#include "atom/utils/to_string.hpp"

void CommandDispatcher::checkPrecondition(const Command& cmd,
                                          const std::string& name) {
    LOG_SCOPE_FUNCTION(INFO);
    if (!cmd.precondition.has_value()) {
        LOG_F(INFO, "No precondition for command: {}", name);
        return;
    }
    try {
        std::invoke(cmd.precondition.value());
    } catch (const std::bad_function_call& e) {
        LOG_F(INFO, "Bad precondition function invoke: {}", e.what());
    } catch (const std::bad_optional_access& e) {
        LOG_F(INFO, "Bad precondition function access: {}", e.what());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Precondition failed: {}", e.what());
        THROW_DISPATCH_EXCEPTION("Precondition failed: " +
                                 std::string(e.what()));
    }
}

void CommandDispatcher::checkPostcondition(const Command& cmd,
                                           const std::string& name) {
    LOG_SCOPE_FUNCTION(INFO);
    if (!cmd.postcondition.has_value()) {
        LOG_F(INFO, "No postcondition for command: {}", name);
        return;
    }
    try {
        std::invoke(cmd.postcondition.value());
    } catch (const std::bad_function_call& e) {
        LOG_F(INFO, "Bad postcondition function invoke: {}", e.what());
    } catch (const std::bad_optional_access& e) {
        LOG_F(INFO, "Bad postcondition function access: {}", e.what());
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Postcondition failed: {}", e.what());
        THROW_DISPATCH_EXCEPTION("Postcondition failed: " + std::string(e.what()));
    }
}

auto CommandDispatcher::executeCommand(
    const Command& cmd, const std::string& name,
    const std::vector<std::any>& args) -> std::any {
    LOG_SCOPE_FUNCTION(INFO);
    if (auto timeoutIt = timeoutMap_.find(name);
        timeoutIt != timeoutMap_.end()) {
        LOG_F(INFO, "Executing command with timeout: {}", name);
        return executeWithTimeout(cmd, name, args, timeoutIt->second);
    }
    LOG_F(INFO, "Executing command without timeout: {}", name);
    return executeWithoutTimeout(cmd, name, args);
}

auto CommandDispatcher::executeWithTimeout(
    const Command& cmd, const std::string& name,
    const std::vector<std::any>& args,
    const std::chrono::duration<double>& timeout) -> std::any {
    LOG_SCOPE_FUNCTION(INFO);
    auto future = std::async(std::launch::async,
                             [&]() { return executeFunctions(cmd, args); });

    if (future.wait_for(timeout) == std::future_status::timeout) {
        LOG_F(ERROR, "Command timed out: {}", name);
        THROW_DISPATCH_TIMEOUT("Command timed out: " + name);
    }

    return future.get();
}

auto CommandDispatcher::executeWithoutTimeout(
    const Command& cmd, [[maybe_unused]] const std::string& name,
    const std::vector<std::any>& args) -> std::any {
    LOG_SCOPE_FUNCTION(INFO);
    if (!args.empty()) {
        if (args.size() == 1 &&
            args[0].type() == typeid(std::vector<std::any>)) {
            LOG_F(INFO, "Executing command with nested arguments: {}", name);
            return executeFunctions(
                cmd, std::any_cast<std::vector<std::any>>(args[0]));
        }
    }

    LOG_F(INFO, "Executing command with arguments: {}", name);
    return executeFunctions(cmd, args);
}

auto CommandDispatcher::executeFunctions(
    const Command& cmd, const std::vector<std::any>& args) -> std::any {
    LOG_SCOPE_FUNCTION(INFO);
    if (std::string funcHash = computeFunctionHash(args);
        cmd.hash == funcHash) {
        try {
            LOG_F(INFO, "Executing function with hash: {}", funcHash);
            return std::invoke(cmd.func, args);
        } catch (const std::bad_any_cast&) {
            LOG_F(ERROR, "Failed to call function with hash: {}", funcHash);
            THROW_DISPATCH_EXCEPTION("Failed to call function with hash " +
                                     funcHash);
        }
    }

    LOG_F(ERROR, "No matching overload found for command");
    THROW_INVALID_ARGUMENT("No matching overload found");
}

auto CommandDispatcher::computeFunctionHash(const std::vector<std::any>& args)
    -> std::string {
    LOG_SCOPE_FUNCTION(INFO);
    std::vector<std::string> argTypes;
    argTypes.reserve(args.size());
    for (const auto& arg : args) {
        argTypes.emplace_back(
            atom::meta::DemangleHelper::demangle(arg.type().name()));
    }
    return atom::utils::toString(atom::algorithm::computeHash(argTypes));
}

auto CommandDispatcher::has(const std::string& name) const -> bool {
    LOG_SCOPE_FUNCTION(INFO);
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

void CommandDispatcher::addAlias(const std::string& name,
                                 const std::string& alias) {
    LOG_SCOPE_FUNCTION(INFO);
    auto it = commands_.find(name);
    if (it != commands_.end()) {
        it->second.aliases.insert(alias);
        commands_[alias] = it->second;
        groupMap_[alias] = groupMap_[name];
    }
}

void CommandDispatcher::addGroup(const std::string& name,
                                 const std::string& group) {
    LOG_SCOPE_FUNCTION(INFO);
    groupMap_[name] = group;
}

void CommandDispatcher::setTimeout(const std::string& name,
                                   std::chrono::milliseconds timeout) {
    LOG_SCOPE_FUNCTION(INFO);
    timeoutMap_[name] = timeout;
}

void CommandDispatcher::removeCommand(const std::string& name) {
    LOG_SCOPE_FUNCTION(INFO);
    commands_.erase(name);
    groupMap_.erase(name);
    timeoutMap_.erase(name);
}

auto CommandDispatcher::getCommandsInGroup(const std::string& group) const
    -> std::vector<std::string> {
    LOG_SCOPE_FUNCTION(INFO);
    std::vector<std::string> result;
    for (const auto& pair : groupMap_) {
        if (pair.second == group) {
            result.push_back(pair.first);
        }
    }
    return result;
}

auto CommandDispatcher::getCommandDescription(const std::string& name) const
    -> std::string {
    LOG_SCOPE_FUNCTION(INFO);
    auto it = commands_.find(name);
    if (it != commands_.end()) {
        return it->second.description;
    }
    return "";
}

auto CommandDispatcher::getCommandAliases(const std::string& name) const
    -> std::unordered_set<std::string> {
    LOG_SCOPE_FUNCTION(INFO);
    auto it = commands_.find(name);
    if (it != commands_.end()) {
        return it->second.aliases;
    }
    return {};
}

auto CommandDispatcher::dispatch(
    const std::string& name, const std::vector<std::any>& args) -> std::any {
    LOG_SCOPE_FUNCTION(INFO);
    return dispatchHelper(name, args);
}

auto CommandDispatcher::dispatch(const std::string& name,
                                 const atom::meta::FunctionParams& params)
    -> std::any {
    LOG_SCOPE_FUNCTION(INFO);
    return dispatchHelper(name, params.toAnyVector());
}

auto CommandDispatcher::getAllCommands() const -> std::vector<std::string> {
    LOG_SCOPE_FUNCTION(INFO);
    std::vector<std::string> result;
    result.reserve(commands_.size());
    for (const auto& pair : commands_) {
        result.push_back(pair.first);
    }
    for (const auto& command : commands_) {
        for (const auto& alias : command.second.aliases) {
            result.push_back(alias);
        }
    }
    auto it = std::unique(result.begin(), result.end());
    result.erase(it, result.end());
    return result;
}

auto CommandDispatcher::getCommandArgAndReturnType(const std::string& name)
    -> std::pair<std::vector<atom::meta::Arg>, std::string> {
    LOG_SCOPE_FUNCTION(INFO);
    auto it = commands_.find(name);
    if (it != commands_.end()) {
        return {it->second.argTypes, it->second.returnType};
    }
    return {{}, ""};
}