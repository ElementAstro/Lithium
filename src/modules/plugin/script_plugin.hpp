#ifndef SCRIPT_PLUGIN_H
#define SCRIPT_PLUGIN_H

#include "plugin.hpp"

#include "modules/system/process.hpp"

class ScriptPlugin : public Plugin
{
public:
    ScriptPlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description, std::shared_ptr<Lithium::Process::ProcessManager> processManager);

    void Execute(const std::vector<std::string> &args) const override;
private:
    std::shared_ptr<Lithium::Process::ProcessManager> m_ProcessManager;
};

#endif
