#pragma once

#include <map>
#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <stdexcept>

#include "plugin.hpp"

namespace Lithium
{
    class PluginManager
    {
    public:
        void LoadPlugin(const std::string &PluginName, std::string PluginPath, std::string version, std::string author, std::string description);
        void UnloadPlugin(const std::string &PluginName);
        void RunPlugin(const std::string &PluginName, const std::vector<std::string> &args);
        void ListPlugins() const;
        void GetPluginInfo(const std::string &PluginName) const;

    private:
        std::map<std::string, std::shared_ptr<Plugin>> Plugins_;
        mutable std::mutex mutex_;
    };

}
