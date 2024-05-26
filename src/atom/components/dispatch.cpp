#include "dispatch.hpp"

bool CommandDispatcher::has(const std::string& name) const {
    if (commands.find(name) != commands.end())
        return true;
    for (const auto& command : commands) {
        if (command.second.aliases.find(name) != command.second.aliases.end())
            return true;
    }
    return false;
}

void CommandDispatcher::addAlias(const std::string& name,
                                 const std::string& alias) {
    auto it = commands.find(name);
    if (it != commands.end()) {
        it->second.aliases.insert(alias);
        commands[alias] = it->second;
        groupMap[alias] = groupMap[name];
    }
}

void CommandDispatcher::addGroup(const std::string& name,
                                 const std::string& group) {
    groupMap[name] = group;
}

void CommandDispatcher::setTimeout(const std::string& name,
                                   std::chrono::milliseconds timeout) {
    timeoutMap[name] = timeout;
}

void CommandDispatcher::clearCache() { cacheMap.clear(); }

void CommandDispatcher::removeCommand(const std::string& name) {
    commands.erase(name);
    groupMap.erase(name);
    timeoutMap.erase(name);
    cacheMap.erase(name);
}

std::vector<std::string> CommandDispatcher::getCommandsInGroup(
    const std::string& group) const {
    std::vector<std::string> result;
    for (const auto& pair : groupMap) {
        if (pair.second == group) {
            result.push_back(pair.first);
        }
    }
    return result;
}

std::string CommandDispatcher::getCommandDescription(
    const std::string& name) const {
    auto it = commands.find(name);
    if (it != commands.end()) {
        return it->second.description;
    }
    return "";
}

std::unordered_set<std::string> CommandDispatcher::getCommandAliases(
    const std::string& name) const {
    auto it = commands.find(name);
    if (it != commands.end()) {
        return it->second.aliases;
    }
    return {};
}

std::any CommandDispatcher::dispatch(const std::string& name,
                                     const std::vector<std::any>& args) {
    return dispatchHelper(name, args);
}

std::vector<std::string> CommandDispatcher::getAllCommands() const {
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
