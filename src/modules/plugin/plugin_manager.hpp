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
    /**
     * @class PluginManager
     * @brief 插件管理器类，用于加载、卸载和运行插件
     */
    class PluginManager
    {
    public:
        /**
         * @brief 构造函数，初始化插件管理器
         */
        PluginManager();

        /**
         * @brief 创建并返回共享的PluginManager指针
         * @return 共享的PluginManager指针
         */
        static std::shared_ptr<PluginManager> CreateShared();

        /**
         * @brief 加载插件
         * @param pluginName 插件名称
         * @param pluginPath 插件路径
         * @param version 插件版本
         * @param author 作者
         * @param description 插件描述
         * @param type 插件类型
         */
        void LoadPlugin(const std::string &pluginName, const std::string &pluginPath, const std::string &version, const std::string &author, const std::string &description, const std::string &type);

        /**
         * @brief 卸载插件
         * @param pluginName 插件名称
         */
        void UnloadPlugin(const std::string &pluginName);

        /**
         * @brief 运行插件
         * @param pluginName 插件名称
         * @param args 运行参数
         */
        void RunPlugin(const std::string &pluginName, const std::vector<std::string> &args);

        /**
         * @brief 列出已加载的插件
         */
        void ListPlugins() const;

        /**
         * @brief 获取指定插件的信息
         * @param pluginName 插件名称
         */
        void GetPluginInfo(const std::string &pluginName) const;

    private:
        mutable std::mutex mutex_;                                              ///< 互斥锁，用于保护plugins_的访问
        std::map<std::string, std::shared_ptr<Plugin>> plugins_;                ///< 已加载的插件容器，以插件名称为键值存储
        std::string GetPluginType(const std::shared_ptr<Plugin> &plugin) const; ///< 获取插件类型的辅助函数
    };

}
