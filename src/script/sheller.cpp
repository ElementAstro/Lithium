/*
 * sheller.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-1-13

Description: System Script Manager

**************************************************/

#include "sheller.hpp"

#include <chrono>
#include <future>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <vector>

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

namespace lithium {

class ScriptManagerImpl {
    using ScriptMap = std::unordered_map<std::string, Script>;
    ScriptMap scripts_;
    ScriptMap powerShellScripts_;
    std::unordered_map<std::string, std::vector<Script>> scriptVersions_;
    std::unordered_map<std::string, std::function<bool()>> scriptConditions_;
    std::unordered_map<std::string, std::string> executionEnvironments_;

    std::unordered_map<std::string, std::string> scriptOutputs_;
    std::unordered_map<std::string, int> scriptStatus_;
    mutable std::shared_mutex mSharedMutex_;

    auto runScriptImpl(std::string_view name,
                       const std::unordered_map<std::string, std::string>& args,
                       bool safe, std::optional<int> timeoutMs)
        -> std::optional<std::pair<std::string, int>>;

public:
    void registerScript(std::string_view name, const Script& script);
    void registerPowerShellScript(std::string_view name, const Script& script);
    auto getAllScripts() const -> ScriptMap;
    void deleteScript(std::string_view name);
    void updateScript(std::string_view name, const Script& script);

    auto runScript(std::string_view name,
                   const std::unordered_map<std::string, std::string>& args,
                   bool safe = true,
                   std::optional<int> timeoutMs = std::nullopt)
        -> std::optional<std::pair<std::string, int>>;
    auto getScriptOutput(std::string_view name) const
        -> std::optional<std::string>;
    auto getScriptStatus(std::string_view name) const -> std::optional<int>;

    auto runScriptsSequentially(
        const std::vector<std::pair<
            std::string, std::unordered_map<std::string, std::string>>>&
            scripts,
        bool safe = true)
        -> std::vector<std::optional<std::pair<std::string, int>>>;
    auto runScriptsConcurrently(
        const std::vector<std::pair<
            std::string, std::unordered_map<std::string, std::string>>>&
            scripts,
        bool safe = true)
        -> std::vector<std::optional<std::pair<std::string, int>>>;

    void enableVersioning();
    auto rollbackScript(std::string_view name, int version) -> bool;

    void setScriptCondition(std::string_view name,
                            std::function<bool()> condition);
    void setExecutionEnvironment(std::string_view name,
                                 const std::string& environment);
};

ScriptManager::ScriptManager()
    : pImpl_(std::make_unique<ScriptManagerImpl>()) {}
ScriptManager::~ScriptManager() = default;

void ScriptManager::registerScript(std::string_view name,
                                   const Script& script) {
    pImpl_->registerScript(name, script);
}

void ScriptManager::registerPowerShellScript(std::string_view name,
                                             const Script& script) {
    pImpl_->registerPowerShellScript(name, script);
}

auto ScriptManager::getAllScripts() const
    -> std::unordered_map<std::string, Script> {
    return pImpl_->getAllScripts();
}

void ScriptManager::deleteScript(std::string_view name) {
    pImpl_->deleteScript(name);
}

void ScriptManager::updateScript(std::string_view name, const Script& script) {
    pImpl_->updateScript(name, script);
}

auto ScriptManager::runScript(
    std::string_view name,
    const std::unordered_map<std::string, std::string>& args, bool safe,
    std::optional<int> timeoutMs)
    -> std::optional<std::pair<std::string, int>> {
    return pImpl_->runScript(name, args, safe, timeoutMs);
}

auto ScriptManager::getScriptOutput(std::string_view name) const
    -> std::optional<std::string> {
    return pImpl_->getScriptOutput(name);
}

auto ScriptManager::getScriptStatus(std::string_view name) const
    -> std::optional<int> {
    return pImpl_->getScriptStatus(name);
}

auto ScriptManager::runScriptsSequentially(
    const std::vector<std::pair<
        std::string, std::unordered_map<std::string, std::string>>>& scripts,
    bool safe) -> std::vector<std::optional<std::pair<std::string, int>>> {
    return pImpl_->runScriptsSequentially(scripts, safe);
}

auto ScriptManager::runScriptsConcurrently(
    const std::vector<std::pair<
        std::string, std::unordered_map<std::string, std::string>>>& scripts,
    bool safe) -> std::vector<std::optional<std::pair<std::string, int>>> {
    return pImpl_->runScriptsConcurrently(scripts, safe);
}

void ScriptManager::enableVersioning() { pImpl_->enableVersioning(); }

auto ScriptManager::rollbackScript(std::string_view name, int version) -> bool {
    return pImpl_->rollbackScript(name, version);
}

void ScriptManager::setScriptCondition(std::string_view name,
                                       std::function<bool()> condition) {
    pImpl_->setScriptCondition(name, condition);
}

void ScriptManager::setExecutionEnvironment(std::string_view name,
                                            const std::string& environment) {
    pImpl_->setExecutionEnvironment(name, environment);
}

void ScriptManagerImpl::registerScript(std::string_view name,
                                       const Script& script) {
    std::unique_lock lock(mSharedMutex_);
    scripts_[std::string(name)] = script;
    if (scriptVersions_.contains(std::string(name))) {
        scriptVersions_[std::string(name)].push_back(script);
    }
}

void ScriptManagerImpl::registerPowerShellScript(std::string_view name,
                                                 const Script& script) {
    std::unique_lock lock(mSharedMutex_);
    powerShellScripts_[std::string(name)] = script;
    if (scriptVersions_.contains(std::string(name))) {
        scriptVersions_[std::string(name)].push_back(script);
    }
}

auto ScriptManagerImpl::getAllScripts() const -> ScriptMap {
    std::shared_lock lock(mSharedMutex_);
    ScriptMap allScripts = scripts_;
    allScripts.insert(powerShellScripts_.begin(), powerShellScripts_.end());
    return allScripts;
}

void ScriptManagerImpl::deleteScript(std::string_view name) {
    std::unique_lock lock(mSharedMutex_);
    scripts_.erase(std::string(name));
    powerShellScripts_.erase(std::string(name));
    scriptOutputs_.erase(std::string(name));
    scriptStatus_.erase(std::string(name));
    scriptVersions_.erase(std::string(name));
    scriptConditions_.erase(std::string(name));
    executionEnvironments_.erase(std::string(name));
}

void ScriptManagerImpl::updateScript(std::string_view name,
                                     const Script& script) {
    std::unique_lock lock(mSharedMutex_);
    auto nameStr = std::string(name);
    if (scripts_.contains(nameStr)) {
        scripts_[nameStr] = script;
    } else if (powerShellScripts_.contains(nameStr)) {
        powerShellScripts_[nameStr] = script;
    } else {
        return;
    }
    if (scriptVersions_.contains(nameStr)) {
        scriptVersions_[nameStr].push_back(script);
    }
    scriptOutputs_[nameStr] = "";
    scriptStatus_[nameStr] = 0;
}

auto ScriptManagerImpl::runScriptImpl(
    std::string_view name,
    const std::unordered_map<std::string, std::string>& args, bool safe,
    std::optional<int> timeoutMs)
    -> std::optional<std::pair<std::string, int>> {
    std::unique_lock lock(mSharedMutex_);
    if (scriptConditions_.contains(std::string(name)) &&
        !scriptConditions_[std::string(name)]()) {
        return std::nullopt;
    }

    std::string scriptCmd;
    if (scripts_.contains(std::string(name))) {
        scriptCmd = "sh -c \"" + scripts_[std::string(name)] + "\"";
    } else if (powerShellScripts_.contains(std::string(name))) {
        scriptCmd = "powershell.exe -Command \"" +
                    powerShellScripts_[std::string(name)] + "\"";
    } else {
        return std::nullopt;
    }

    for (const auto& arg : args) {
        scriptCmd += " \"" + arg.first + "=" + arg.second + "\"";
    }

    if (executionEnvironments_.contains(std::string(name))) {
        scriptCmd = executionEnvironments_[std::string(name)] + " " + scriptCmd;
    }

    auto future = std::async(std::launch::async, [scriptCmd] {
        return atom::system::executeCommandWithStatus(scriptCmd);
    });

    std::optional<std::pair<std::string, int>> result;
    if (timeoutMs.has_value()) {
        if (future.wait_for(std::chrono::milliseconds(*timeoutMs)) ==
            std::future_status::timeout) {
            result =
                std::make_optional<std::pair<std::string, int>>("Timeout", -1);
        } else {
            result = future.get();
        }
    } else {
        result = future.get();
    }

    if (result.has_value()) {
        scriptOutputs_[std::string(name)] = result->first;
        scriptStatus_[std::string(name)] = result->second;
    }

    return result;
}

auto ScriptManagerImpl::runScript(
    std::string_view name,
    const std::unordered_map<std::string, std::string>& args, bool safe,
    std::optional<int> timeoutMs)
    -> std::optional<std::pair<std::string, int>> {
    return runScriptImpl(name, args, safe, timeoutMs);
}

auto ScriptManagerImpl::getScriptOutput(std::string_view name) const
    -> std::optional<std::string> {
    std::shared_lock lock(mSharedMutex_);
    if (scriptOutputs_.contains(std::string(name))) {
        return scriptOutputs_.at(std::string(name));
    }
    return std::nullopt;
}

auto ScriptManagerImpl::getScriptStatus(std::string_view name) const
    -> std::optional<int> {
    std::shared_lock lock(mSharedMutex_);
    if (scriptStatus_.contains(std::string(name))) {
        return scriptStatus_.at(std::string(name));
    }
    return std::nullopt;
}

auto ScriptManagerImpl::runScriptsSequentially(
    const std::vector<std::pair<
        std::string, std::unordered_map<std::string, std::string>>>& scripts,
    bool safe) -> std::vector<std::optional<std::pair<std::string, int>>> {
    std::vector<std::optional<std::pair<std::string, int>>> results;
    results.reserve(scripts.size());
    for (const auto& [name, args] : scripts) {
        results.push_back(runScriptImpl(name, args, safe, std::nullopt));
    }
    return results;
}

auto ScriptManagerImpl::runScriptsConcurrently(
    const std::vector<std::pair<
        std::string, std::unordered_map<std::string, std::string>>>& scripts,
    bool safe) -> std::vector<std::optional<std::pair<std::string, int>>> {
    std::vector<std::future<std::optional<std::pair<std::string, int>>>>
        futures;
    futures.reserve(scripts.size());
    for (const auto& [name, args] : scripts) {
        futures.push_back(std::async(std::launch::async,
                                     &ScriptManagerImpl::runScriptImpl, this,
                                     name, args, safe, std::nullopt));
    }
    std::vector<std::optional<std::pair<std::string, int>>> results;
    results.reserve(futures.size());
for (auto& future : futures) {
        results.push_back(future.get());
    }
    return results;
}

void ScriptManagerImpl::enableVersioning() {
    std::unique_lock lock(mSharedMutex_);
    for (auto& [name, script] : scripts_) {
        scriptVersions_[name] = {script};
    }
    for (auto& [name, script] : powerShellScripts_) {
        scriptVersions_[name] = {script};
    }
}

auto ScriptManagerImpl::rollbackScript(std::string_view name,
                                       int version) -> bool {
    std::unique_lock lock(mSharedMutex_);
    auto nameStr = std::string(name);
    if (!scriptVersions_.contains(nameStr) || version < 0 ||
        version >= static_cast<int>(scriptVersions_[nameStr].size())) {
        return false;
    }
    if (scripts_.contains(nameStr)) {
        scripts_[nameStr] = scriptVersions_[nameStr][version];
    } else if (powerShellScripts_.contains(nameStr)) {
        powerShellScripts_[nameStr] = scriptVersions_[nameStr][version];
    } else {
        return false;
    }
    scriptOutputs_[nameStr] = "";
    scriptStatus_[nameStr] = 0;
    return true;
}

void ScriptManagerImpl::setScriptCondition(std::string_view name,
                                           std::function<bool()> condition) {
    std::unique_lock lock(mSharedMutex_);
    scriptConditions_[std::string(name)] = std::move(condition);
}

void ScriptManagerImpl::setExecutionEnvironment(
    std::string_view name, const std::string& environment) {
    std::unique_lock lock(mSharedMutex_);
    executionEnvironments_[std::string(name)] = environment;
}

}  // namespace lithium
