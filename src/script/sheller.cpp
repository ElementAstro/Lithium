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
#include <sstream>


#ifdef _WIN32
const std::string SHELL_COMMAND = "powershell.exe -Command";
#else
const std::string SHELL_COMMAND = "sh -c";
#endif

#include "atom/log/loguru.hpp"

namespace Lithium {
void ScriptManager::RegisterScript(const std::string &name,
                                   const Script &script) {
    if (scripts.find(name) != scripts.end()) {
        LOG_F(ERROR, "Script already registered: {}", name);
        return;
    }
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    scripts[name] = script;
    scriptOutputs[name] = "";
    scriptStatus[name] = 0;
    DLOG_F(INFO, "Script registered: {}", name);
}

void ScriptManager::RegisterPowerShellScript(const std::string &name,
                                             const Script &script) {
    if (powerShellScripts.find(name) != powerShellScripts.end()) {
        LOG_F(ERROR, "PowerShell script already registered: {}", name);
        return;
    }
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    powerShellScripts[name] = script;
    scriptOutputs[name] = "";
    scriptStatus[name] = 0;
    DLOG_F(INFO, "PowerShell script registered: {}", name);
}

void ScriptManager::ViewScripts() {
    if (scripts.empty() && powerShellScripts.empty()) {
        LOG_F(INFO, "No scripts registered.");
    } else {
        std::shared_lock<std::shared_mutex> lock(m_sharedMutex);
        LOG_F(INFO, "Registered scripts:");
        for (const auto &pair : scripts) {
            LOG_F(INFO, "{}", pair.first);
        }
        for (const auto &pair : powerShellScripts) {
            LOG_F(INFO, "{} ps", pair.first);
        }
    }
}

void ScriptManager::DeleteScript(const std::string &name) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    auto it = scripts.find(name);
    if (it != scripts.end()) {
        scripts.erase(it);
        scriptOutputs.erase(name);
        scriptStatus.erase(name);
        LOG_F(INFO, "Script deleted: {}", name);
    } else {
        auto it2 = powerShellScripts.find(name);
        if (it2 != powerShellScripts.end()) {
            powerShellScripts.erase(it2);
            scriptOutputs.erase(name);
            scriptStatus.erase(name);
            LOG_F(ERROR, "PowerShell script not found: {}", name);
        } else {
            LOG_F(ERROR, "Script not found: {}", name);
        }
    }
}

void ScriptManager::UpdateScript(const std::string &name,
                                 const Script &script) {
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    auto it = scripts.find(name);
    if (it != scripts.end()) {
        it->second = script;
        scriptOutputs[name] = "";
        scriptStatus[name] = 0;
        LOG_F(INFO, "Script updated: {}", name);
    } else {
        auto it2 = powerShellScripts.find(name);
        if (it2 != powerShellScripts.end()) {
            it2->second = script;
            scriptOutputs[name] = "";
            scriptStatus[name] = 0;
            LOG_F(INFO, "PowerShell script updated: {}", name);
        } else {
            LOG_F(ERROR, "PowerShell script not found: {}", name);
        }
    }
}

bool ScriptManager::RunScript(const std::string &name,
                              const std::vector<std::string> &args) {
    if (scripts.empty() && powerShellScripts.empty()) {
        LOG_F(ERROR, "No scripts registered.");
        return false;
    }
    std::unique_lock<std::shared_mutex> lock(m_sharedMutex);
    auto it = scripts.find(name);
    if (it != scripts.end()) {
        // 执行Shell脚本
        const std::string &script = it->second;
        std::string cmdLine = SHELL_COMMAND + " \"" + script + "\"";
        return RunCommand(cmdLine, name, args);
    } else {
        auto it2 = powerShellScripts.find(name);
        if (it2 != powerShellScripts.end()) {
            // 执行PowerShell脚本
            const std::string &script = it2->second;
            std::string cmdLine = "powershell.exe -Command \"" + script + "\"";
            return RunCommand(cmdLine, name, args);
        } else {
            LOG_F(ERROR, "PowerShell script not found: {}", name);
            return false;
        }
    }
}

void ScriptManager::ViewScriptOutput(const std::string &name) {
    auto it = scriptOutputs.find(name);
    if (it != scriptOutputs.end()) {
        const std::string &output = it->second;
        LOG_F(INFO, "Output of script {}: {}", name, output);
    } else {
        LOG_F(ERROR, "Script not found: {}", name);
    }
}

void ScriptManager::ViewScriptStatus(const std::string &name) {
    auto it = scriptStatus.find(name);
    if (it != scriptStatus.end()) {
        int status = it->second;
        LOG_F(INFO, "Status of script {}: {}", name, status);
    } else {
        LOG_F(ERROR, "Script not found: {}", name);
    }
}

bool ScriptManager::RunCommand(const std::string &cmdLine,
                               const std::string &name,
                               const std::vector<std::string> &args) {
    // 构建命令行参数
    std::string fullCmdLine = cmdLine;
    for (const std::string &arg : args) {
        fullCmdLine += " \"" + arg + "\"";
    }

    // 执行命令
    std::stringstream outputBuffer;
    FILE *pipe = popen(fullCmdLine.c_str(), "r");
    if (pipe != nullptr) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            outputBuffer << buffer;
        }

        std::string output = outputBuffer.str();
        scriptOutputs[name] = output;

        int result = pclose(pipe);
        scriptStatus[name] = result;

        if (result != 0) {
            LogError("Script run error: " + name);
        }

        return (result == 0);
    } else {
        LogError("Script run error: " + name);
        return false;
    }
}

void ScriptManager::LogError(const std::string &message) {
    // 记录错误日志
    std::time_t now = std::time(nullptr);
    std::tm *timeInfo = std::localtime(&now);
    char buffer[80];
    std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeInfo);

    std::ofstream logFile("error.log", std::ios_base::app);
    logFile << "[" << buffer << "] " << message << std::endl;
    logFile.close();
}

#if ENABLE_PEGTL

#endif
}  // namespace Lithium
