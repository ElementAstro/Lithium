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
#include <functional>

#include "lidriver/core/device.hpp"
#include "config/configor.hpp"
#include "module/modloader.hpp"
#include "server/message_bus.hpp"

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

    class DeviceManager
    {
    public:
        DeviceManager(std::shared_ptr<MessageBus> messageBus, std::shared_ptr<Config::ConfigManager> configManager);

        ~DeviceManager();

        static std::shared_ptr<DeviceManager> createShared(std::shared_ptr<MessageBus> messageBus, std::shared_ptr<Config::ConfigManager> configManager);

        std::vector<std::string> getDeviceList(DeviceType type);

        bool addDevice(DeviceType type, const std::string &name, const std::string &lib_name);

        bool addDeviceLibrary(const std::string &lib_path, const std::string &lib_name);

        bool AddDeviceObserver(DeviceType type, const std::string &name);

        bool removeDevice(DeviceType type, const std::string &name);

        bool removeDevicesByName(const std::string &name);

        bool removeDeviceLibrary(const std::string &lib_name);

        std::shared_ptr<Device> getDevice(DeviceType type, const std::string &name);

        size_t findDevice(DeviceType type, const std::string &name);

        std::shared_ptr<Device> findDeviceByName(const std::string &name) const;

        std::shared_ptr<SimpleTask> getTask(DeviceType type, const std::string &device_name, const std::string &task_name, const nlohmann::json &params);

        void messageBusPublishString(const std::shared_ptr<IStringProperty> &message);

        void messageBusPublishNumber(const std::shared_ptr<INumberProperty> &message);

        void messageBusPublishBool(const std::shared_ptr<IBoolProperty> &message);

    private:
        std::vector<std::shared_ptr<Device>> m_devices[static_cast<int>(DeviceType::NumDeviceTypes)]; ///< An array of vectors of shared pointers to Device objects, one for each DeviceType.

        std::mutex m_mutex;

        std::shared_ptr<ModuleLoader> m_ModuleLoader;
        std::shared_ptr<MessageBus> m_MessageBus;
        std::shared_ptr<Config::ConfigManager> m_ConfigManager;
    };

}