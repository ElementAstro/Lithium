#ifndef CHAISCRIPT_PLUGIN_H
#define CHAISCRIPT_PLUGIN_H

#include "plugin.hpp"


class ChaiScriptPlugin : public Plugin
{
public:
    ChaiScriptPlugin(const std::string &path, const std::string &version, const std::string &author, const std::string &description);

    void Execute(const std::vector<std::string> &args) const override;
};

#endif
