#include "dispatch.hpp"

#include "atom/log/loguru.hpp"
#include "atom/utils/to_string.hpp"

Arg::Arg(std::string name) : name_(std::move(name)) {}

Arg::Arg(std::string name, std::any default_value)
    : name_(std::move(name)), default_value_(default_value) {}

auto Arg::getName() const -> const std::string& { return name_; }

auto Arg::getDefaultValue() const -> const std::optional<std::any>& {
    return default_value_;
}

void CommandDispatcher::checkPrecondition(const Command& cmd,
                                          const std::string& name) {
    LOG_SCOPE_FUNCTION(INFO);
    if (cmd.precondition && !cmd.precondition.value()()) {
        LOG_F(ERROR, "Precondition failed for command: {}", name);
        THROW_DISPATCH_EXCEPTION("Precondition failed for command: " + name);
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
    if (cmd.funcs.size() == 1) {
        LOG_F(INFO, "Executing single function for command");
        return cmd.funcs[0](args);
    }

    std::string funcHash = computeFunctionHash(args);
    for (size_t i = 0; i < cmd.funcs.size(); ++i) {
        if (cmd.hash[i] == funcHash) {
            try {
                LOG_F(INFO, "Executing function with hash: {}", funcHash);
                return cmd.funcs[i](args);
            } catch (const std::bad_any_cast&) {
                LOG_F(ERROR, "Failed to call function with hash: {}", funcHash);
                THROW_DISPATCH_EXCEPTION("Failed to call function with hash " +
                                         funcHash);
            }
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
                                 const FunctionParams& params) -> std::any {
    LOG_SCOPE_FUNCTION(INFO);
    return dispatchHelper(name, params.toVector());
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
