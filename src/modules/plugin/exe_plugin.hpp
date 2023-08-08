#ifndef EXECUTABLE_PLUGIN_H
#define EXECUTABLE_PLUGIN_H

#include "plugin.hpp"

#include "modules/system/process.hpp"

class ExecutablePlugin : public Plugin
{
public:
    ExecutablePlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description, std::shared_ptr<Lithium::Process::ProcessManager> processManager);

    void Execute(const std::vector<std::string> &args) const override;
private:
    std::shared_ptr<Lithium::Process::ProcessManager> m_ProcessManager;
};

#endif
