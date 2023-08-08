#ifndef SCRIPT_PLUGIN_H
#define SCRIPT_PLUGIN_H

#include "plugin.hpp"

class ScriptPlugin : public Plugin
{
public:
    ScriptPlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description);

    void Execute(const std::vector<std::string> &args) const override;
};

#endif
