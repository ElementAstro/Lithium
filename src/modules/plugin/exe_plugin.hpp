#ifndef EXECUTABLE_PLUGIN_H
#define EXECUTABLE_PLUGIN_H

#include "plugin.hpp"

class ExecutablePlugin : public Plugin
{
public:
    ExecutablePlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description);

    void Execute(const std::vector<std::string> &args) const override;
};

#endif
