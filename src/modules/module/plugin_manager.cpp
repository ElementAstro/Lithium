#include "plugin_manager.hpp"

#include "loguru/loguru.hpp"

namespace Lithium
{
    PluginManager::PluginManager()
    {
    }

    std::shared_ptr<PluginManager> PluginManager::createShared()
    {
        return std::make_shared<PluginManager>();
    }

    void PluginManager::LoadPlugin(const std::string &PluginName, std::string PluginPath, std::string version, std::string author, std::string description)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (Plugins_.contains(PluginName))
        {
            LOG_F(ERROR, "Error: Plugin '%s' already exists.", PluginName.c_str());
            throw std::runtime_error("Error: Plugin '" + PluginName + "' already exists.");
        }

        // 加载工具
        // ...

        Plugins_.emplace(PluginName, std::make_shared<Plugin>(std::move(PluginPath), std::move(version), std::move(author), std::move(description)));

        LOG_F(INFO, "Plugin '%s' loaded successfully.", PluginName.c_str());
    }

    void PluginManager::UnloadPlugin(const std::string &PluginName)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = Plugins_.find(PluginName);
        if (it != Plugins_.end())
        {
            // 卸载工具
            // ...

            Plugins_.erase(it);
            LOG_F(INFO, "Plugin '%s' unloaded successfully.", PluginName.c_str());
        }
        else
        {
            LOG_F(ERROR, "Error: Plugin '%s' does not exist.", PluginName.c_str());
            throw std::runtime_error("Error: Plugin '" + PluginName + "' does not exist.");
        }
    }

    void PluginManager::RunPlugin(const std::string &PluginName, const std::vector<std::string> &args)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = Plugins_.find(PluginName);
        if (it != Plugins_.end())
        {
            const std::shared_ptr<Plugin> &Plugin = it->second;
            std::string command = Plugin->GetPath();

            // 添加参数到命令行
            for (const std::string &arg : args)
            {
                command += " " + arg;
            }

            LOG_F(INFO, "Running Plugin '%s' with args:", PluginName.c_str());
            for (const std::string &arg : args)
            {
                LOG_F(INFO, "- %s", arg.c_str());
            }

            // 执行命令
            std::system(command.c_str());
        }
        else
        {
            LOG_F(ERROR, "Error: Plugin '%s' does not exist.", PluginName.c_str());
            throw std::runtime_error("Error: Plugin '" + PluginName + "' does not exist.");
        }
    }

    void PluginManager::ListPlugins() const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        LOG_F(INFO, "Loaded Plugins:");
        for (const auto &[PluginName, Plugin] : Plugins_)
        {
            LOG_F(INFO, "- Name: %s", PluginName.c_str());
            LOG_F(INFO, "  Path: %s", Plugin->GetPath().c_str());
            LOG_F(INFO, "  Version: %s", Plugin->GetVersion().c_str());
            LOG_F(INFO, "  Author: %s", Plugin->GetAuthor().c_str());
            LOG_F(INFO, "  Description: %s", Plugin->GetDescription().c_str());
        }
    }

    void PluginManager::GetPluginInfo(const std::string &PluginName) const
    {
        std::lock_guard<std::mutex> lock(mutex_);

        auto it = Plugins_.find(PluginName);
        if (it != Plugins_.end())
        {
            const std::shared_ptr<Plugin> &Plugin = it->second;
            LOG_F(INFO, "Plugin info for '%s':", PluginName.c_str());
            LOG_F(INFO, "- Path: %s", Plugin->GetPath().c_str());
            LOG_F(INFO, "- Version: %s", Plugin->GetVersion().c_str());
            LOG_F(INFO, "- Author: %s", Plugin->GetAuthor().c_str());
            LOG_F(INFO, "- Description: %s", Plugin->GetDescription().c_str());
        }
        else
        {
            LOG_F(ERROR, "Error: Plugin '%s' does not exist.", PluginName.c_str());
            throw std::runtime_error("Error: Plugin '" + PluginName + "' does not exist.");
        }
    }

} // namespace Lithium::Plugin
