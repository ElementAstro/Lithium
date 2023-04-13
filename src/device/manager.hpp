/*
 * manager.hpp
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

#include "basic_device.hpp"

#include <string>
#include <vector>
#include <mutex>
#include <functional>

namespace OpenAPT {

    /**
     * @brief The DeviceManager class manages a collection of devices and provides methods for device management.
     *
     * DeviceManager类管理一组设备，并提供了设备管理的方法。
     */
    class DeviceManager {
    public:
        /**
         * @brief Constructs a DeviceManager object.
         *
         * 构造一个DeviceManager对象。
         */
        DeviceManager();

        /**
         * @brief Destroys the DeviceManager object.
         *
         * 销毁DeviceManager对象。
         */
        ~DeviceManager();

        /**
         * @brief Gets a list of device names of the specified type.
         *
         * 获取指定类型的设备名称列表。
         *
         * @param type The type of the device to get the list of.
         * @return A vector of device names of the specified type.
         */
        std::vector<std::string> getDeviceList(DeviceType type);

        /**
         * @brief Adds a new device of the specified type with the specified name.
         *
         * 添加指定名称和类型的新设备。
         *
         * @param type The type of the device to add.
         * @param name The name of the device to add.
         */
        void addDevice(DeviceType type, const std::string& name);

        /**
         * @brief Removes the device with the specified name and type.
         *
         * 移除指定名称和类型的设备。
         *
         * @param type The type of the device to remove.
         * @param name The name of the device to remove.
         */
        void removeDevice(DeviceType type, const std::string& name);

        /**
         * @brief Removes all devices with the specified name.
         *
         * 移除所有指定名称的设备。
         *
         * @param name The name of the devices to remove.
         */
        void removeDevicesByName(const std::string& name);

        /**
         * @brief Gets the device with the specified name and type.
         *
         * 获取指定名称和类型的设备。
         *
         * @param type The type of the device to get.
         * @param name The name of the device to get.
         * @return A shared pointer to the device with the specified name and type, or nullptr if not found.
         */
        std::shared_ptr<Device> getDevice(DeviceType type, const std::string& name);

        /**
         * @brief Finds the index of the device with the specified name and type.
         *
         * 查找指定名称和类型的设备索引。
         *
         * @param type The type of the device to find.
         * @param name The name of the device to find.
         * @return The index of the device with the specified name and type, or -1 if not found.
         */
        size_t findDevice(DeviceType type, const std::string& name);

        /**
         * @brief Finds the device with the specified name.
         *
         * 查找指定名称的设备。
         *
         * @param name The name of the device to find.
         * @return A shared pointer to the device with the specified name, or nullptr if not found.
         */
        std::shared_ptr<Device> findDeviceByName(const std::string& name) const;

        std::shared_ptr<Camera> getCamera(const std::string& name);

        std::shared_ptr<SimpleTask> getSimpleTask(DeviceType type,const std::string& device_type,const std::string& device_name,const std::string& task_name);

        std::shared_ptr<ConditionalTask> getConditionalTask(const std::string& device_name, const std::string& task_name);

        std::shared_ptr<LoopTask> getLoopTask(const std::string& device_name,const std::string task_name);

    private:
        std::vector<std::shared_ptr<Device>> m_devices[6]; ///< An array of vectors of shared pointers to Device objects, one for each DeviceType.
    
        std::mutex m_mutex;
    };

}