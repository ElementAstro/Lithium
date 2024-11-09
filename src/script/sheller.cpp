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
#include <sstream>
#include <stdexcept>
#include <utility>
#include <vector>

#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

namespace lithium {

/**
 * @brief Custom exception for script-related errors.
 */
class ScriptException : public std::runtime_error {
public:
    explicit ScriptException(const std::string& message)
        : std::runtime_error(message) {}
};

class ScriptManagerImpl {
    using ScriptMap = std::unordered_map<std::string, Script>;
    ScriptMap scripts_;
    ScriptMap powerShellScripts_;
    std::unordered_map<std::string, std::vector<Script>> scriptVersions_;
    std::unordered_map<std::string, std::function<bool()>> scriptConditions_;
    std::unordered_map<std::string, std::string> executionEnvironments_;
    std::unordered_map<std::string, std::vector<std::string>> scriptLogs_;

    std::unordered_map<std::string, std::string> scriptOutputs_;
    std::unordered_map<std::string, int> scriptStatus_;
    mutable std::shared_mutex mSharedMutex_;

    int maxVersions_ = 10;

    auto runScriptImpl(std::string_view name,
                       const std::unordered_map<std::string, std::string>& args,
                       bool safe, std::optional<int> timeoutMs, int retryCount)
        -> std::optional<std::pair<std::string, int>>;

public:
    void registerScript(std::string_view name, const Script& script);
    void registerPowerShellScript(std::string_view name, const Script& script);
    auto getAllScripts() const -> ScriptMap;
    void deleteScript(std::string_view name);
    void updateScript(std::string_view name, const Script& script);

    auto runScript(
        std::string_view name,
        const std::unordered_map<std::string, std::string>& args,
        bool safe = true, std::optional<int> timeoutMs = std::nullopt,
        int retryCount = 0) -> std::optional<std::pair<std::string, int>>;
    auto getScriptOutput(std::string_view name) const
        -> std::optional<std::string>;
    auto getScriptStatus(std::string_view name) const -> std::optional<int>;

    auto runScriptsSequentially(
        const std::vector<std::pair<
            std::string, std::unordered_map<std::string, std::string>>>&
            scripts,
        bool safe = true, int retryCount = 0)
        -> std::vector<std::optional<std::pair<std::string, int>>>;
    auto runScriptsConcurrently(
        const std::vector<std::pair<
            std::string, std::unordered_map<std::string, std::string>>>&
            scripts,
        bool safe = true, int retryCount = 0)
        -> std::vector<std::optional<std::pair<std::string, int>>>;

    void enableVersioning();
    auto rollbackScript(std::string_view name, int version) -> bool;

    void setScriptCondition(std::string_view name,
                            std::function<bool()> condition);
    void setExecutionEnvironment(std::string_view name,
                                 const std::string& environment);
    void setMaxScriptVersions(int maxVersions);
    auto getScriptLogs(std::string_view name) const -> std::vector<std::string>;
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
    std::optional<int> timeoutMs,
    int retryCount) -> std::optional<std::pair<std::string, int>> {
    return pImpl_->runScript(name, args, safe, timeoutMs, retryCount);
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
    bool safe,
    int retryCount) -> std::vector<std::optional<std::pair<std::string, int>>> {
    return pImpl_->runScriptsSequentially(scripts, safe, retryCount);
}

auto ScriptManager::runScriptsConcurrently(
    const std::vector<std::pair<
        std::string, std::unordered_map<std::string, std::string>>>& scripts,
    bool safe,
    int retryCount) -> std::vector<std::optional<std::pair<std::string, int>>> {
    return pImpl_->runScriptsConcurrently(scripts, safe, retryCount);
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

void ScriptManager::setMaxScriptVersions(int maxVersions) {
    pImpl_->setMaxScriptVersions(maxVersions);
}

auto ScriptManager::getScriptLogs(std::string_view name) const
    -> std::vector<std::string> {
    return pImpl_->getScriptLogs(name);
}

// Implementation of ScriptManagerImpl

void ScriptManagerImpl::registerScript(std::string_view name,
                                       const Script& script) {
    std::unique_lock lock(mSharedMutex_);
    std::string nameStr(name);
    scripts_[nameStr] = script;
    if (scriptVersions_.contains(nameStr)) {
        scriptVersions_[nameStr].push_back(script);
        if (scriptVersions_[nameStr].size() >
            static_cast<size_t>(maxVersions_)) {
            scriptVersions_[nameStr].erase(scriptVersions_[nameStr].begin());
        }
    } else {
        scriptVersions_[nameStr] = {script};
    }
    scriptLogs_[nameStr].emplace_back("Script registered/updated.");
}

void ScriptManagerImpl::registerPowerShellScript(std::string_view name,
                                                 const Script& script) {
    std::unique_lock lock(mSharedMutex_);
    std::string nameStr(name);
    powerShellScripts_[nameStr] = script;
    if (scriptVersions_.contains(nameStr)) {
        scriptVersions_[nameStr].push_back(script);
        if (scriptVersions_[nameStr].size() >
            static_cast<size_t>(maxVersions_)) {
            scriptVersions_[nameStr].erase(scriptVersions_[nameStr].begin());
        }
    } else {
        scriptVersions_[nameStr] = {script};
    }
    scriptLogs_[nameStr].emplace_back("PowerShell script registered/updated.");
}

auto ScriptManagerImpl::getAllScripts() const -> ScriptMap {
    std::shared_lock lock(mSharedMutex_);
    ScriptMap allScripts = scripts_;
    allScripts.insert(powerShellScripts_.begin(), powerShellScripts_.end());
    return allScripts;
}

void ScriptManagerImpl::deleteScript(std::string_view name) {
    std::unique_lock lock(mSharedMutex_);
    std::string nameStr(name);
    auto erased = scripts_.erase(nameStr) + powerShellScripts_.erase(nameStr);
    if (erased == 0) {
        throw ScriptException("Script not found: " + nameStr);
    }
    scriptOutputs_.erase(nameStr);
    scriptStatus_.erase(nameStr);
    scriptVersions_.erase(nameStr);
    scriptConditions_.erase(nameStr);
    executionEnvironments_.erase(nameStr);
    scriptLogs_.erase(nameStr);
    LOG_F(INFO, "Script deleted: %s", nameStr.c_str());
}

void ScriptManagerImpl::updateScript(std::string_view name,
                                     const Script& script) {
    std::unique_lock lock(mSharedMutex_);
    std::string nameStr(name);
    if (scripts_.contains(nameStr)) {
        scripts_[nameStr] = script;
    } else if (powerShellScripts_.contains(nameStr)) {
        powerShellScripts_[nameStr] = script;
    } else {
        throw ScriptException("Script not found for update: " + nameStr);
    }
    if (scriptVersions_.contains(nameStr)) {
        scriptVersions_[nameStr].push_back(script);
        if (scriptVersions_[nameStr].size() >
            static_cast<size_t>(maxVersions_)) {
            scriptVersions_[nameStr].erase(scriptVersions_[nameStr].begin());
        }
    } else {
        scriptVersions_[nameStr] = {script};
    }
    scriptOutputs_[nameStr] = "";
    scriptStatus_[nameStr] = 0;
    scriptLogs_[nameStr].emplace_back("Script updated.");
}

auto ScriptManagerImpl::runScriptImpl(
    std::string_view name,
    const std::unordered_map<std::string, std::string>& args, bool safe,
    std::optional<int> timeoutMs,
    int retryCount) -> std::optional<std::pair<std::string, int>> {
    std::string nameStr(name);
    {
        std::shared_lock lock(mSharedMutex_);
        if (scriptConditions_.contains(nameStr) &&
            !scriptConditions_[nameStr]()) {
            LOG_F(WARNING,
                  "Condition for script '%s' not met. Skipping execution.",
                  nameStr.c_str());
            scriptLogs_[nameStr].emplace_back(
                "Script execution skipped due to condition.");
            return std::nullopt;
        }
    }

    int attempts = 0;
    while (attempts <= retryCount) {
        std::string scriptCmd;
        {
            std::shared_lock lock(mSharedMutex_);
            if (scripts_.contains(nameStr)) {
                scriptCmd = "sh -c \"" + scripts_[nameStr] + "\"";
            } else if (powerShellScripts_.contains(nameStr)) {
                scriptCmd = "powershell.exe -Command \"" +
                            powerShellScripts_[nameStr] + "\"";
            } else {
                throw ScriptException("Script not found: " + nameStr);
            }

            for (const auto& arg : args) {
                scriptCmd += " \"" + arg.first + "=" + arg.second + "\"";
            }

            if (executionEnvironments_.contains(nameStr)) {
                scriptCmd = executionEnvironments_[nameStr] + " " + scriptCmd;
            }
        }

        auto future = std::async(std::launch::async, [scriptCmd]() {
            return atom::system::executeCommandWithStatus(scriptCmd);
        });

        std::optional<std::pair<std::string, int>> result;
        if (timeoutMs.has_value()) {
            if (future.wait_for(std::chrono::milliseconds(*timeoutMs)) ==
                std::future_status::timeout) {
                result = std::make_optional<std::pair<std::string, int>>(
                    "Timeout", -1);
                LOG_F(ERROR, "Script '%s' execution timed out.",
                      nameStr.c_str());
            } else {
                result = future.get();
            }
        } else {
            result = future.get();
        }

        {
            std::unique_lock lock(mSharedMutex_);
            if (result.has_value()) {
                scriptOutputs_[nameStr] = result->first;
                scriptStatus_[nameStr] = result->second;
                scriptLogs_[nameStr].emplace_back(
                    "Script executed successfully.");
                return result;
            } else {
                scriptLogs_[nameStr].emplace_back(
                    "Script execution failed or timed out.");
            }
        }

        attempts++;
        if (attempts <= retryCount) {
            LOG_F(WARNING, "Retrying script '%s' (%d/%d).", nameStr.c_str(),
                  attempts, retryCount);
            scriptLogs_[nameStr].emplace_back("Retrying script execution.");
        }
    }

    scriptLogs_[nameStr].emplace_back("Script execution failed after retries.");
    return std::nullopt;
}

auto ScriptManagerImpl::runScript(
    std::string_view name,
    const std::unordered_map<std::string, std::string>& args, bool safe,
    std::optional<int> timeoutMs,
    int retryCount) -> std::optional<std::pair<std::string, int>> {
    try {
        return runScriptImpl(name, args, safe, timeoutMs, retryCount);
    } catch (const ScriptException& e) {
        LOG_F(ERROR, "ScriptException: %s", e.what());
        throw;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Exception during script execution: %s", e.what());
        throw ScriptException("Unknown error during script execution.");
    }
}

auto ScriptManagerImpl::getScriptOutput(std::string_view name) const
    -> std::optional<std::string> {
    std::shared_lock lock(mSharedMutex_);
    std::string nameStr(name);
    if (scriptOutputs_.contains(nameStr)) {
        return scriptOutputs_.at(nameStr);
    }
    return std::nullopt;
}

auto ScriptManagerImpl::getScriptStatus(std::string_view name) const
    -> std::optional<int> {
    std::shared_lock lock(mSharedMutex_);
    std::string nameStr(name);
    if (scriptStatus_.contains(nameStr)) {
        return scriptStatus_.at(nameStr);
    }
    return std::nullopt;
}

auto ScriptManagerImpl::runScriptsSequentially(
    const std::vector<std::pair<
        std::string, std::unordered_map<std::string, std::string>>>& scripts,
    bool safe,
    int retryCount) -> std::vector<std::optional<std::pair<std::string, int>>> {
    std::vector<std::optional<std::pair<std::string, int>>> results;
    results.reserve(scripts.size());
    for (const auto& [name, args] : scripts) {
        try {
            results.emplace_back(
                runScriptImpl(name, args, safe, std::nullopt, retryCount));
        } catch (const ScriptException& e) {
            LOG_F(ERROR, "Error running script '%s': %s", name.c_str(),
                  e.what());
            results.emplace_back(std::nullopt);
        }
    }
    return results;
}

auto ScriptManagerImpl::runScriptsConcurrently(
    const std::vector<std::pair<
        std::string, std::unordered_map<std::string, std::string>>>& scripts,
    bool safe,
    int retryCount) -> std::vector<std::optional<std::pair<std::string, int>>> {
    std::vector<std::future<std::optional<std::pair<std::string, int>>>>
        futures;
    futures.reserve(scripts.size());
    for (const auto& [name, args] : scripts) {
        futures.emplace_back(
            std::async(std::launch::async, &ScriptManagerImpl::runScriptImpl,
                       this, name, args, safe, std::nullopt, retryCount));
    }
    std::vector<std::optional<std::pair<std::string, int>>> results;
    results.reserve(futures.size());
    for (auto& future : futures) {
        try {
            results.emplace_back(future.get());
        } catch (const ScriptException& e) {
            LOG_F(ERROR, "ScriptException during concurrent execution: %s",
                  e.what());
            results.emplace_back(std::nullopt);
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception during concurrent execution: %s", e.what());
            results.emplace_back(std::nullopt);
        }
    }
    return results;
}

void ScriptManagerImpl::enableVersioning() {
    std::unique_lock lock(mSharedMutex_);
    for (auto& [name, script] : scripts_) {
        scriptVersions_[name].push_back(script);
        if (scriptVersions_[name].size() > static_cast<size_t>(maxVersions_)) {
            scriptVersions_[name].erase(scriptVersions_[name].begin());
        }
    }
    for (auto& [name, script] : powerShellScripts_) {
        scriptVersions_[name].push_back(script);
        if (scriptVersions_[name].size() > static_cast<size_t>(maxVersions_)) {
            scriptVersions_[name].erase(scriptVersions_[name].begin());
        }
    }
    LOG_F(INFO, "Versioning enabled for all scripts.");
}

auto ScriptManagerImpl::rollbackScript(std::string_view name,
                                       int version) -> bool {
    std::unique_lock lock(mSharedMutex_);
    std::string nameStr(name);
    if (!scriptVersions_.contains(nameStr) || version < 0 ||
        version >= static_cast<int>(scriptVersions_[nameStr].size())) {
        LOG_F(ERROR, "Invalid rollback attempt for script '%s' to version %d.",
              nameStr.c_str(), version);
        return false;
    }
    if (scripts_.contains(nameStr)) {
        scripts_[nameStr] = scriptVersions_[nameStr][version];
    } else if (powerShellScripts_.contains(nameStr)) {
        powerShellScripts_[nameStr] = scriptVersions_[nameStr][version];
    } else {
        LOG_F(ERROR, "Script '%s' not found for rollback.", nameStr.c_str());
        return false;
    }
    scriptOutputs_[nameStr] = "";
    scriptStatus_[nameStr] = 0;
    scriptLogs_[nameStr].emplace_back("Script rolled back to version " +
                                      std::to_string(version) + ".");
    return true;
}

void ScriptManagerImpl::setScriptCondition(std::string_view name,
                                           std::function<bool()> condition) {
    std::unique_lock lock(mSharedMutex_);
    scriptConditions_[std::string(name)] = std::move(condition);
    scriptLogs_[std::string(name)].emplace_back("Script condition set.");
}

void ScriptManagerImpl::setExecutionEnvironment(
    std::string_view name, const std::string& environment) {
    std::unique_lock lock(mSharedMutex_);
    executionEnvironments_[std::string(name)] = environment;
    scriptLogs_[std::string(name)].emplace_back("Execution environment set.");
}

void ScriptManagerImpl::setMaxScriptVersions(int maxVersions) {
    std::unique_lock lock(mSharedMutex_);
    maxVersions_ = maxVersions;
    for (auto& [name, versions] : scriptVersions_) {
        while (versions.size() > static_cast<size_t>(maxVersions_)) {
            versions.erase(versions.begin());
        }
    }
    LOG_F(INFO, "Max script versions set to %d.", maxVersions_);
}

auto ScriptManagerImpl::getScriptLogs(std::string_view name) const
    -> std::vector<std::string> {
    std::shared_lock lock(mSharedMutex_);
    std::string nameStr(name);
    if (scriptLogs_.contains(nameStr)) {
        return scriptLogs_.at(nameStr);
    }
    return {};
}

}  // namespace lithium
