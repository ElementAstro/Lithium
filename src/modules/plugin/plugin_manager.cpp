#include "plugin_manager.hpp"

#include "exe_plugin.hpp"
#include "script_plugin.hpp"
#include "chai_plugin.hpp"

#include "loguru/loguru.hpp"

namespace Lithium
{
    PluginManager::PluginManager()
    {
    }

    std::shared_ptr<PluginManager> PluginManager::CreateShared()
    {
        return std::make_shared<PluginManager>();
    }

    void PluginManager::LoadPlugin(const std::string &pluginName, const std::string &pluginPath, const std::string &version, const std::string &author, const std::string &description, const std::string &type)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (plugins_.count(pluginName))
        {
            LOG_F(ERROR, "Error: Plugin '%s' already exists.", pluginName.c_str());
            throw std::runtime_error("Error: Plugin '" + pluginName + "' already exists.");
        }

        std::shared_ptr<Plugin> plugin;

        if (type == "ScriptPlugin")
        {
            plugin = std::make_shared<ScriptPlugin>(pluginPath, version, author, description);
        }
        else if (type == "ExecutablePlugin")
        {
            plugin = std::make_shared<ExecutablePlugin>(pluginPath, version, author, description);
        }
        else if (type == "ChaiScriptPlugin")
        {
            plugin = std::make_shared<ChaiScriptPlugin>(pluginPath, version, author, description);
        }
        else
        {
            LOG_F(ERROR, "Error: Unknown plugin type '%s'.", type.c_str());
            throw std::runtime_error("Error: Unknown plugin type '" + type + "'.");
        }

        plugins_[pluginName] = plugin;

        LOG_F(INFO, "Plugin '%s' loaded successfully.", pluginName.c_str());
    }

    void PluginManager::UnloadPlugin(const std::string &pluginName)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = plugins_.find(pluginName);
        if (it != plugins_.end())
        {
            plugins_.erase(it);
            LOG_F(INFO, "Plugin '%s' unloaded successfully.", pluginName.c_str());
        }
        else
        {
            LOG_F(ERROR, "Error: Plugin '%s' does not exist.", pluginName.c_str());
            throw std::runtime_error("Error: Plugin '" + pluginName + "' does not exist.");
        }
    }

    void PluginManager::RunPlugin(const std::string &pluginName, const std::vector<std::string> &args)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = plugins_.find(pluginName);
        if (it != plugins_.end())
        {
            const std::shared_ptr<Plugin> &plugin = it->second;

            LOG_F(INFO, "Running Plugin '%s' with args:", pluginName.c_str());
            for (const std::string &arg : args)
            {
                LOG_F(INFO, "- %s", arg.c_str());
            }

            plugin->Execute(args);
        }
        else
        {
            LOG_F(ERROR, "Error: Plugin '%s' does not exist.", pluginName.c_str());
            throw std::runtime_error("Error: Plugin '" + pluginName + "' does not exist.");
        }
    }

    void PluginManager::ListPlugins() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        LOG_F(INFO, "Loaded Plugins:");
        for (const auto &pair : plugins_)
        {
            const std::string &pluginName = pair.first;
            const std::shared_ptr<Plugin> &plugin = pair.second;
            LOG_F(INFO, "- Name: %s", pluginName.c_str());
            LOG_F(INFO, "  Path: %s", plugin->GetPath().c_str());
            LOG_F(INFO, "  Type: %s", GetPluginType(plugin).c_str());
            LOG_F(INFO, "  Version: %s", plugin->GetVersion().c_str());
            LOG_F(INFO, "  Author: %s", plugin->GetAuthor().c_str());
            LOG_F(INFO, "  Description: %s", plugin->GetDescription().c_str());
        }
    }

    void PluginManager::GetPluginInfo(const std::string &pluginName) const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = plugins_.find(pluginName);
        if (it != plugins_.end())
        {
            const std::shared_ptr<Plugin> &plugin = it->second;
            LOG_F(INFO, "Plugin info for '%s':", pluginName.c_str());
            LOG_F(INFO, "- Path: %s", plugin->GetPath().c_str());
            LOG_F(INFO, "- Type: %s", GetPluginType(plugin).c_str());
            LOG_F(INFO, "- Version: %s", plugin->GetVersion().c_str());
            LOG_F(INFO, "- Author: %s", plugin->GetAuthor().c_str());
            LOG_F(INFO, "- Description: %s", plugin->GetDescription().c_str());
        }
        else
        {
            LOG_F(ERROR, "Error: Plugin '%s' does not exist.", pluginName.c_str());
            throw std::runtime_error("Error: Plugin '" + pluginName + "' does not exist.");
        }
    }

    std::string PluginManager::GetPluginType(const std::shared_ptr<Plugin> &plugin) const
    {
        if (dynamic_cast<const ScriptPlugin *>(plugin.get()))
        {
            return "ScriptPlugin";
        }
        else if (dynamic_cast<const ExecutablePlugin *>(plugin.get()))
        {
            return "ExecutablePlugin";
        }
        else if (dynamic_cast<const ChaiScriptPlugin *>(plugin.get()))
        {
            return "ChaiScriptPlugin";
        }
        else
        {
            return "Unknown";
        }
    }

} // namespace Lithium::Plugin
