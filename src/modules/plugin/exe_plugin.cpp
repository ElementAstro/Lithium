#include "exe_plugin.hpp"

#include "loguru/loguru.hpp"

#include <fstream>
#include <sstream>

ExecutablePlugin::ExecutablePlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description, std::shared_ptr<Lithium::Process::ProcessManager> processManager)
    : Plugin(path, version, author, description)
{
    m_ProcessManager = processManager;
}

void ExecutablePlugin::Execute(const std::vector<std::string> &args) const
{
    std::ostringstream oss;
    oss << GetPath();
    for (const std::string &arg : args)
    {
        oss << " " << arg;
    }
    std::string command = oss.str();
    LOG_F(INFO, "Running command: %s", command.c_str());
    if (m_ProcessManager)
    {
        if (!m_ProcessManager->createProcess(command, GetPath()))
        {
            LOG_F(ERROR, "Failed to run executable plugin : %s", command.c_str());
        }
        else
        {
            LOG_F(ERROR, "Started %s successfully", command.c_str());
        }
    }
    else
    {
        LOG_F(ERROR, "Process manager is not initialized");
    }
}
