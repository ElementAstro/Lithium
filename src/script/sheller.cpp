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

bool ScriptManager::registerCommon(
    std::unordered_map<std::string, std::string>& scriptMap,
    std::string_view name, const std::string& script) {
    std::unique_lock lock(m_sharedMutex_);
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

ScriptMap ScriptManager::getAllScripts() const {
    std::shared_lock lock(m_sharedMutex_);
    ScriptMap scripts_;
    for (const auto& [name, script] : powerShellScripts_) {
        scripts_[name] = script;
    }
    for (const auto& [name, script] : scripts_) {
        scripts_[name] = script;
    }
    return scripts_;
}

void ScriptManager::deleteScript(std::string_view name) {
    std::unique_lock lock(m_sharedMutex_);
    auto nameStr = std::string(name);
    if (auto it = scripts_.find(nameStr); it != scripts_.end()) {
        scripts_.erase(it);
        scriptOutputs_.erase(nameStr);
        scriptStatus_.erase(nameStr);
        LOG_F(INFO, "Script deleted: {}", name);
    } else {
        if (auto it2 = powerShellScripts_.find(nameStr);
            it2 != powerShellScripts_.end()) {
            powerShellScripts_.erase(it2);
            scriptOutputs_.erase(nameStr);
            scriptStatus_.erase(nameStr);
            LOG_F(ERROR, "PowerShell script not found: {}", name);
        } else {
            LOG_F(ERROR, "Script not found: {}", name);
        }
    }
}

void ScriptManager::updateScript(std::string_view name, const Script& script) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex_);
    std::string nameStr{name};

    auto [it, success] = scripts_.insert_or_assign(nameStr, script);
    if (success) {
        scriptOutputs_[nameStr] = "";
        scriptStatus_[nameStr] = 0;
        LOG_F(INFO, "Script updated: {}", nameStr);
        return;
    }

    auto [it2, success2] = powerShellScripts_.insert_or_assign(nameStr, script);
    if (success2) {
        scriptOutputs_[nameStr] = "";
        scriptStatus_[nameStr] = 0;
        LOG_F(INFO, "PowerShell script updated: {}", nameStr);
    } else {
        LOG_F(ERROR, "Script not found: {}", nameStr);
    }
}

bool ScriptManager::runScript(
    std::string_view name,
    const std::unordered_map<std::string, std::string>& args) {
    std::unique_lock lock(m_sharedMutex_);
    std::string scriptCmd;

    // 尝试从普通脚本中找到并执行
    if (auto it = scripts_.find(std::string(name)); it != scripts_.end()) {
        scriptCmd = SHELL_COMMAND + " \"" + it->second + "\"";
    }
    // 如果不是普通脚本，尝试从PowerShell脚本中找到并执行
    else if (auto it2 = powerShellScripts_.find(std::string(name));
             it2 != powerShellScripts_.end()) {
        scriptCmd = "powershell.exe -Command \"" + it2->second + "\"";
    }
    // 如果两者都没有找到，返回错误
    else {
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
