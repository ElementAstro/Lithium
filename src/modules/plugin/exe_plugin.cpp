#include "exe_plugin.hpp"
#include <iostream>

ExecutablePlugin::ExecutablePlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description)
    : Plugin(path, version, author, description)
{
}

void ExecutablePlugin::Execute(const std::vector<std::string> &args) const
{
    std::cout << "Running ExecutablePlugin with args:" << std::endl;
    for (const std::string &arg : args)
    {
        std::cout << "- " << arg << std::endl;
    }
}
