/*
 * plugin_loader.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*************************************************

Date: 2023-7-13

Description: Plugin Manager

**************************************************/

#pragma once

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <stdexcept>

#if ENABLE_FASTHASH
#include "emhash/hash_table8.hpp"
#else
#include <unordered_map>
#endif

#include "plugin.hpp"

namespace Lithium
{
    namespace Process
    {
        class ProcessManager;
    } // namespace Process

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
        PluginManager(std::shared_ptr<Process::ProcessManager> processManager);

        // -------------------------------------------------------------------
        // Common methods
        // -------------------------------------------------------------------

        /**
         * @brief 创建并返回共享的PluginManager指针
         * @return 共享的PluginManager指针
         */
        static std::shared_ptr<PluginManager> createShared(std::shared_ptr<Process::ProcessManager> processManager);

        static std::unique_ptr<PluginManager> createUnique(std::shared_ptr<Process::ProcessManager> processManager);

        

        // ---------------------------------------------------------------------
        // Plugin methods
        // ---------------------------------------------------------------------

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
        mutable std::mutex mutex_; ///< 互斥锁，用于保护plugins_的访问
#if ENABLE_FASTHASH
        emhash8::HashMap<std::string, std::shared_ptr<Plugin>> plugins_;
#else
        std::unordered_map<std::string, std::shared_ptr<Plugin>> plugins_; ///< 已加载的插件容器，以插件名称为键值存储
#endif

        std::string GetPluginType(const std::shared_ptr<Plugin> &plugin) const; ///< 获取插件类型的辅助函数

        std::shared_ptr<Process::ProcessManager> m_ProcessManager;
    };

}
