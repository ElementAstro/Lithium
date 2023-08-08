#include "script_plugin.hpp"
#include <iostream>

ScriptPlugin::ScriptPlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description)
    : Plugin(path, version, author, description)
{
}

void ScriptPlugin::Execute(const std::vector<std::string> &args) const
{
    std::cout << "Running ScriptPlugin with args:" << std::endl;
    for (const std::string &arg : args)
    {
        std::cout << "- " << arg << std::endl;
    }
}
