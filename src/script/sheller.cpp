#include "sheller.hpp"

#ifdef _WIN32
const std::string SHELL_COMMAND = "powershell.exe -Command";
#else
const std::string SHELL_COMMAND = "sh -c";
#endif

namespace Lithium::Script
{
    void ScriptManager::RegisterScript(const std::string &name, const Script &script)
    {
        scripts[name] = script;
        scriptOutputs[name] = "";
        scriptStatus[name] = 0;
        std::cout << "Script registered: " << name << std::endl;
    }

    void ScriptManager::RegisterPowerShellScript(const std::string &name, const Script &script)
    {
        powerShellScripts[name] = script;
        scriptOutputs[name] = "";
        scriptStatus[name] = 0;
        std::cout << "PowerShell script registered: " << name << std::endl;
    }

    void ScriptManager::ViewScripts()
    {
        if (scripts.empty() && powerShellScripts.empty())
        {
            std::cout << "No scripts registered." << std::endl;
        }
        else
        {
            std::cout << "Registered scripts:" << std::endl;
            for (const auto &pair : scripts)
            {
                std::cout << pair.first << std::endl;
            }
            for (const auto &pair : powerShellScripts)
            {
                std::cout << pair.first << " (PowerShell)" << std::endl;
            }
        }
    }

    void ScriptManager::DeleteScript(const std::string &name)
    {
        auto it = scripts.find(name);
        if (it != scripts.end())
        {
            scripts.erase(it);
            scriptOutputs.erase(name);
            scriptStatus.erase(name);
            std::cout << "Script deleted: " << name << std::endl;
        }
        else
        {
            auto it2 = powerShellScripts.find(name);
            if (it2 != powerShellScripts.end())
            {
                powerShellScripts.erase(it2);
                scriptOutputs.erase(name);
                scriptStatus.erase(name);
                std::cout << "PowerShell script deleted: " << name << std::endl;
            }
            else
            {
                std::cout << "Script not found: " << name << std::endl;
            }
        }
    }

    void ScriptManager::UpdateScript(const std::string &name, const Script &script)
    {
        auto it = scripts.find(name);
        if (it != scripts.end())
        {
            it->second = script;
            scriptOutputs[name] = "";
            scriptStatus[name] = 0;
            std::cout << "Script updated: " << name << std::endl;
        }
        else
        {
            auto it2 = powerShellScripts.find(name);
            if (it2 != powerShellScripts.end())
            {
                it2->second = script;
                scriptOutputs[name] = "";
                scriptStatus[name] = 0;
                std::cout << "PowerShell script updated: " << name << std::endl;
            }
            else
            {
                std::cout << "Script not found: " << name << std::endl;
            }
        }
    }

    bool ScriptManager::RunScript(const std::string &name, const std::vector<std::string> &args = {})
    {
        auto it = scripts.find(name);
        if (it != scripts.end())
        {
            // 执行Shell脚本
            const std::string &script = it->second;
            std::string cmdLine = SHELL_COMMAND + " \"" + script + "\"";
            return RunCommand(cmdLine, name, args);
        }
        else
        {
            auto it2 = powerShellScripts.find(name);
            if (it2 != powerShellScripts.end())
            {
                // 执行PowerShell脚本
                const std::string &script = it2->second;
                std::string cmdLine = "powershell.exe -Command \"" + script + "\"";
                return RunCommand(cmdLine, name, args);
            }
            else
            {
                std::cout << "Script not found: " << name << std::endl;
                return false;
            }
        }
    }

    void ScriptManager::ViewScriptOutput(const std::string &name)
    {
        auto it = scriptOutputs.find(name);
        if (it != scriptOutputs.end())
        {
            const std::string &output = it->second;
            std::cout << "Output of script " << name << ":" << std::endl;
            std::cout << output << std::endl;
        }
        else
        {
            std::cout << "Script not found: " << name << std::endl;
        }
    }

    void ScriptManager::ViewScriptStatus(const std::string &name)
    {
        auto it = scriptStatus.find(name);
        if (it != scriptStatus.end())
        {
            int status = it->second;
            std::cout << "Status of script " << name << ": " << status << std::endl;
        }
        else
        {
            std::cout << "Script not found: " << name << std::endl;
        }
    }

    bool ScriptManager::RunCommand(const std::string &cmdLine, const std::string &name, const std::vector<std::string> &args)
    {
        // 构建命令行参数
        std::string fullCmdLine = cmdLine;
        for (const std::string &arg : args)
        {
            fullCmdLine += " \"" + arg + "\"";
        }

        // 执行命令
        std::stringstream outputBuffer;
        FILE *pipe = popen(fullCmdLine.c_str(), "r");
        if (pipe != nullptr)
        {
            char buffer[128];
            while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
            {
                outputBuffer << buffer;
            }

            std::string output = outputBuffer.str();
            scriptOutputs[name] = output;

            int result = pclose(pipe);
            scriptStatus[name] = result;

            if (result != 0)
            {
                LogError("Script run error: " + name);
            }

            return (result == 0);
        }
        else
        {
            LogError("Script run error: " + name);
            return false;
        }
    }

    void ScriptManager::LogError(const std::string &message)
    {
        // 记录错误日志
        std::time_t now = std::time(nullptr);
        std::tm *timeInfo = std::localtime(&now);
        char buffer[80];
        std::strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeInfo);

        std::ofstream logFile("error.log", std::ios_base::app);
        logFile << "[" << buffer << "] " << message << std::endl;
        logFile.close();

        std::cerr << message << std::endl;
    }
}

/*
int main()
{
ScriptManager manager;

// 注册Shell脚本
manager.RegisterScript("hello", "echo \"Hello, World!\"");
manager.RegisterScript("sum", "a=$1\nb=$2\nsum=$((a+b))\necho \"Sum: $sum\"");

// 注册PowerShell脚本
manager.RegisterPowerShellScript("greet", R"(
    $name = Read-Host -Prompt "Enter your name"
    Write-Host "Hello, $name!"
)");

// 查看已注册的脚本
manager.ViewScripts();

// 运行脚本
manager.RunScript("hello");
manager.RunScript("sum", {"3", "5"});
manager.RunScript("greet");

// 查看脚本输出和状态
manager.ViewScriptOutput("hello");
manager.ViewScriptStatus("sum");

return 0;
}

*/
