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

#include <fstream>
#include <mutex>
#include <sstream>

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
    registerCommon(scripts, name, script);
}

void ScriptManager::registerPowerShellScript(std::string_view name,
                                             const Script& script) {
    registerCommon(powerShellScripts, name, script);
}

bool ScriptManager::registerCommon(
    std::unordered_map<std::string, std::string>& scriptMap,
    std::string_view name, const std::string& script) {
    std::unique_lock lock(m_sharedMutex);
    auto [it, inserted] = scriptMap.try_emplace(std::string(name), script);
    if (!inserted) {
        LOG_F(ERROR, "Script already registered: {}", std::string(name));
        return false;
    }
    scriptOutputs[std::string(name)] = "";
    scriptStatus[std::string(name)] = 0;
    DLOG_F(INFO, "Script registered: {}", std::string(name));
    return true;
}

ScriptMap ScriptManager::getAllScripts() const {
    std::shared_lock lock(m_sharedMutex);
    ScriptMap _scripts;
    for (const auto& [name, script] : powerShellScripts) {
        _scripts[name] = script;
    }
    for (const auto& [name, script] : scripts) {
        _scripts[name] = script;
    }
    return _scripts;
}

void ScriptManager::deleteScript(std::string_view name) {
    std::unique_lock lock(m_sharedMutex);
    auto name_str = std::string(name);
    if (auto it = scripts.find(name_str); it != scripts.end()) {
        scripts.erase(it);
        scriptOutputs.erase(name_str);
        scriptStatus.erase(name_str);
        LOG_F(INFO, "Script deleted: {}", name);
    } else {
        if (auto it2 = powerShellScripts.find(name_str);
            it2 != powerShellScripts.end()) {
            powerShellScripts.erase(it2);
            scriptOutputs.erase(name_str);
            scriptStatus.erase(name_str);
            LOG_F(ERROR, "PowerShell script not found: {}", name);
        } else {
            LOG_F(ERROR, "Script not found: {}", name);
        }
    }
}

void ScriptManager::updateScript(std::string_view name, const Script& script) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    std::string nameStr{name};

    auto [it, success] = scripts.insert_or_assign(nameStr, script);
    if (success) {
        scriptOutputs[nameStr] = "";
        scriptStatus[nameStr] = 0;
        LOG_F(INFO, "Script updated: {}", nameStr);
        return;
    }

    auto [it2, success2] = powerShellScripts.insert_or_assign(nameStr, script);
    if (success2) {
        scriptOutputs[nameStr] = "";
        scriptStatus[nameStr] = 0;
        LOG_F(INFO, "PowerShell script updated: {}", nameStr);
    } else {
        LOG_F(ERROR, "Script not found: {}", nameStr);
    }
}

bool ScriptManager::runScript(
    std::string_view name,
    const std::unordered_map<std::string, std::string>& args) {
    std::unique_lock lock(m_sharedMutex);
    std::string scriptCmd;

    // 尝试从普通脚本中找到并执行
    if (auto it = scripts.find(std::string(name)); it != scripts.end()) {
        scriptCmd = SHELL_COMMAND + " \"" + it->second + "\"";
    }
    // 如果不是普通脚本，尝试从PowerShell脚本中找到并执行
    else if (auto it2 = powerShellScripts.find(std::string(name));
             it2 != powerShellScripts.end()) {
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
        scriptOutputs[std::string(name)] = result.first;
        scriptStatus[std::string(name)] = result.second;
    } catch (const atom::error::RuntimeError& e) {
        return false;
    }

    return true;
}

}  // namespace lithium
