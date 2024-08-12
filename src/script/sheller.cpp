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

#include <mutex>

#ifdef _WIN32
const std::string SHELL_COMMAND = "powershell.exe -Command";
#else
const std::string SHELL_COMMAND = "sh -c";
#endif

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/system/command.hpp"

namespace lithium {
void ScriptManager::registerScript(std::string_view name,
                                   const Script& script) {
    registerCommon(scripts_, name, script);
}

void ScriptManager::registerPowerShellScript(std::string_view name,
                                             const Script& script) {
    registerCommon(powerShellScripts_, name, script);
}

auto ScriptManager::registerCommon(ScriptMap& scriptMap, std::string_view name,
                                   const std::string& script) -> bool {
    std::unique_lock lock(mSharedMutex_);
    auto [it, inserted] = scriptMap.try_emplace(std::string(name), script);
    if (!inserted) {
        LOG_F(ERROR, "Script already registered: {}", std::string(name));
        return false;
    }
    scriptOutputs_[std::string(name)] = "";
    scriptStatus_[std::string(name)] = 0;
    DLOG_F(INFO, "Script registered: {}", std::string(name));
    return true;
}

auto ScriptManager::getAllScripts() const -> ScriptMap {
    std::shared_lock lock(mSharedMutex_);
    ScriptMap allScripts;
    allScripts.insert(scripts_.begin(), scripts_.end());
    allScripts.insert(powerShellScripts_.begin(), powerShellScripts_.end());
    return allScripts;
}

void ScriptManager::deleteScript(std::string_view name) {
    std::unique_lock lock(mSharedMutex_);
    auto nameStr = std::string(name);
    if ((scripts_.erase(nameStr) != 0U) ||
        (powerShellScripts_.erase(nameStr) != 0U)) {
        scriptOutputs_.erase(nameStr);
        scriptStatus_.erase(nameStr);
        LOG_F(INFO, "Script deleted: {}", name);
    } else {
        LOG_F(ERROR, "Script not found: {}", name);
    }
}

void ScriptManager::updateScript(std::string_view name, const Script& script) {
    std::unique_lock lock(mSharedMutex_);
    auto nameStr = std::string(name);
    if (scripts_.contains(nameStr)) {
        scripts_[nameStr] = script;
    } else if (powerShellScripts_.contains(nameStr)) {
        powerShellScripts_[nameStr] = script;
    } else {
        LOG_F(ERROR, "Script not found: {}", nameStr);
        return;
    }
    scriptOutputs_[nameStr] = "";
    scriptStatus_[nameStr] = 0;
    LOG_F(INFO, "Script updated: {}", nameStr);
}

auto ScriptManager::runScript(
    std::string_view name,
    const std::unordered_map<std::string, std::string>& args) -> bool {
    std::unique_lock lock(mSharedMutex_);
    std::string scriptCmd;

    if (scripts_.contains(std::string(name))) {
        scriptCmd = SHELL_COMMAND + " \"" + scripts_[std::string(name)] + "\"";
    } else if (powerShellScripts_.contains(std::string(name))) {
        scriptCmd = "powershell.exe -Command \"" +
                    powerShellScripts_[std::string(name)] + "\"";
    } else {
        LOG_F(ERROR, "Script not found: {}", name);
        return false;
    }

    for (const auto& arg : args) {
        scriptCmd += " \"" + arg.first + "=" + arg.second + "\"";
    }
    try {
        auto result = atom::system::executeCommandWithStatus(scriptCmd);
        scriptOutputs_[std::string(name)] = result.first;
        scriptStatus_[std::string(name)] = result.second;
    } catch (const atom::error::RuntimeError& e) {
        return false;
    }

    return true;
}

}  // namespace lithium
