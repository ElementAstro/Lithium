/*
 * device_manager.hpp
 *
 * Copyright (C) 2023 Max Qian <lightapt.com>
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

Copyright: 2023 Max Qian. All rights reserved

Author: Max Qian

E-mail: astro_air@126.com

Date: 2023-3-29

Description: Device Manager

**************************************************/

#pragma once

#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <functional>

#include "Hydrogen/core/device.hpp"
#include "config/configor.hpp"
#include "module/modloader.hpp"
#include "server/message_bus.hpp"

#include "Hydrogen/event/eventloop.hpp"

namespace Lithium
{
    enum class DeviceType
    {
        Camera,
        Telescope,
        Focuser,
        FilterWheel,
        Solver,
        Guider,
        NumDeviceTypes
    };
    
    /**
     * @class DeviceManager
     * @brief 设备管理器类，用于管理各种设备对象。
     */
    class DeviceManager
    {
    public:
        /**
         * @brief 构造函数，创建一个设备管理器对象。
         * @param messageBus 消息总线对象的共享指针。
         * @param configManager 配置管理器对象的共享指针。
         */
        DeviceManager(std::shared_ptr<MessageBus> messageBus, std::shared_ptr<Config::ConfigManager> configManager);

        /**
         * @brief 析构函数，销毁设备管理器对象。
         */
        ~DeviceManager();

        /**
         * @brief 创建一个共享的设备管理器对象。
         * @param messageBus 消息总线对象的共享指针。
         * @param configManager 配置管理器对象的共享指针。
         * @return 返回一个指向设备管理器对象的共享指针。
         */
        static std::shared_ptr<DeviceManager> createShared(std::shared_ptr<MessageBus> messageBus, std::shared_ptr<Config::ConfigManager> configManager);

        /**
         * @brief 获取指定类型设备的设备列表。
         * @param type 设备类型枚举值。
         * @return 返回包含设备名称的字符串向量。
         */
        std::vector<std::string> getDeviceList(DeviceType type);

        /**
         * @brief 添加设备到设备管理器中。
         * @param type 设备类型枚举值。
         * @param name 设备名称。
         * @param lib_name 设备库名称。
         * @return 如果添加成功返回true，否则返回false。
         */
        bool addDevice(DeviceType type, const std::string &name, const std::string &lib_name);

        /**
         * @brief 添加设备库到设备管理器中。
         * @param lib_path 设备库路径。
         * @param lib_name 设备库名称。
         * @return 如果添加成功返回true，否则返回false。
         */
        bool addDeviceLibrary(const std::string &lib_path, const std::string &lib_name);

        /**
         * @brief 添加设备观察者。
         * @param type 设备类型枚举值。
         * @param name 设备名称。
         * @return 如果添加成功返回true，否则返回false。
         */
        bool AddDeviceObserver(DeviceType type, const std::string &name);

        /**
         * @brief 从设备管理器中移除指定设备。
         * @param type 设备类型枚举值。
         * @param name 设备名称。
         * @return 如果移除成功返回true，否则返回false。
         */
        bool removeDevice(DeviceType type, const std::string &name);

        /**
         * @brief 根据设备名称从设备管理器中移除设备。
         * @param name 设备名称。
         * @return 如果移除成功返回true，否则返回false。
         */
        bool removeDevicesByName(const std::string &name);

        /**
         * @brief 从设备管理器中移除指定设备库。
         * @param lib_name 设备库名称。
         * @return 如果移除成功返回true，否则返回false。
         */
        bool removeDeviceLibrary(const std::string &lib_name);

        /**
         * @brief 获取指定设备类型和名称的设备对象。
         * @param type 设备类型枚举值。
         * @param name 设备名称。
         * @return 返回指向设备对象的共享指针，如果设备不存在则返回空指针。
         */
        std::shared_ptr<Device> getDevice(DeviceType type, const std::string &name);

        /**
         * @brief 查找指定设备类型和名称的设备在设备管理器中的索引。
         * @param type 设备类型枚举值。
         * @param name 设备名称。
         * @return 返回设备在设备管理器中的索引，如果设备不存在则返回`std::string::npos`。
         */
        size_t findDevice(DeviceType type, const std::string &name);

        /**
         * @brief 根据设备名称查找设备对象。
         * @param name 设备名称。
         * @return 返回指向设备对象的共享指针，如果设备不存在则返回空指针。
         */
        std::shared_ptr<Device> findDeviceByName(const std::string &name) const;

        /**
         * @brief 获取指定设备类型、设备名称、任务名称和参数的简单任务对象。
         * @param type 设备类型枚举值。
         * @param device_name 设备名称。
         * @param task_name 任务名称。
         * @param params 任务参数的JSON对象。
         * @return 返回指向简单任务对象的共享指针。
         */
        std::shared_ptr<SimpleTask> getTask(DeviceType type, const std::string &device_name, const std::string &task_name, const nlohmann::json &params);

        /**
         * @brief 发布字符串类型的消息到消息总线。
         * @param message 字符串属性的共享指针。
         */
        void messageBusPublishString(const std::shared_ptr<IStringProperty> &message);

        /**
         * @brief 发布数字类型的消息到消息总线。
         * @param message 数字属性的共享指针。
         */
        void messageBusPublishNumber(const std::shared_ptr<INumberProperty> &message);

        /**
         * @brief 发布布尔类型的消息到消息总线。
         * @param message 布尔属性的共享指针。
         */
        void messageBusPublishBool(const std::shared_ptr<IBoolProperty> &message);

        /**
         * @brief 设置设备属性值。
         * @param type 设备类型枚举值。
         * @param name 设备名称。
         * @param value_name 属性名称。
         * @param value 属性值。
         * @return 如果设置成功返回true，否则返回错误信息。
         */
        bool setDeviceProperty(DeviceType type, const std::string &name, const std::string &value_name, const std::any &value);

        /**
         * @brief 根据设备名称设置设备属性值。
         * @param name 设备名称。
         * @param value_name 属性名称。
         * @param value 属性值。
         * @return 如果设置成功返回true，否则返回错误信息。
         */
        bool setDevicePropertyByName(const std::string &name, const std::string &value_name, const std::any &value);

    private:
        std::vector<std::shared_ptr<Device>> m_devices[static_cast<int>(DeviceType::NumDeviceTypes)]; ///< 存储设备对象的数组，每个设备类型对应一个向量。

        std::mutex m_mutex; ///< 互斥锁，用于保护设备管理器的并发访问。

        std::shared_ptr<ModuleLoader> m_ModuleLoader;           ///< 模块加载器对象的共享指针。
        std::shared_ptr<MessageBus> m_MessageBus;               ///< 消息总线对象的共享指针。
        std::shared_ptr<EventLoop> m_EventLoop;                 ///< 事件循环对象的共享指针。
        std::shared_ptr<Config::ConfigManager> m_ConfigManager; ///< 配置管理器对象的共享指针。
    };

}