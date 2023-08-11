#include "script_plugin.hpp"

#include "loguru/loguru.hpp"

ScriptPlugin::ScriptPlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description, std::shared_ptr<Lithium::Process::ProcessManager> processManager)
    : Plugin(path, version, author, description)
{
    m_ProcessManager = processManager;
}

void ScriptPlugin::Execute(const std::vector<std::string> &args) const
{
    std::ostringstream oss;
    oss << GetPath();
    for (const std::string &arg : args)
    {
        oss << " " << arg;
    }
    std::string script = oss.str();
    LOG_F(INFO, "Running script: %s", script.c_str());
    if (m_ProcessManager)
    {
        if (!m_ProcessManager->runScript(script, GetPath()))
        {
            LOG_F(ERROR, "Failed to run executable plugin : %s", script.c_str());
        }
        else
        {
            LOG_F(ERROR, "Started %s successfully", script.c_str());
        }
    }
    else
    {
        LOG_F(ERROR, "Process manager is not initialized");
    }
}
