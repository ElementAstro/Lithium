/*
 * device_manager.cpp
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

#include "device_manager.hpp"

#include "nlohmann/json.hpp"

#include "camera.hpp"
#include "telescope.hpp"
#include "focuser.hpp"
#include "filterwheel.hpp"

#include "loguru/loguru.hpp"

#if __cplusplus >= 202002L
#include <format>
#endif

namespace Lithium
{

    // Constructor
    DeviceManager::DeviceManager()
    {
        for (auto &devices : m_devices)
        {
            devices.emplace_back();
        }
    }

    DeviceManager::~DeviceManager()
    {
        for (auto &devices : m_devices)
        {
            for (auto &device : devices)
            {
                if (device)
                {
                    device->disconnect();
                }
            }
        }
    }

    std::vector<std::string> DeviceManager::getDeviceList(DeviceType type)
    {
        std::vector<std::string> deviceList;
        auto &devices = m_devices[static_cast<int>(type)];
        for (const auto &device : devices)
        {
            if (device)
            {
                deviceList.emplace_back(device->getName());
            }
        }
        return deviceList;
    }

    void DeviceManager::addDevice(DeviceType type, const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        // Check if device type is valid
        if (type < DeviceType::Camera || type > DeviceType::Guider)
        {
            throw std::invalid_argument("Invalid device type");
        }

        // Check if device name already exists
        if (findDeviceByName(name))
        {
            // spdlog::warn("A device with name {} already exists, please choose a different name", name);
            return;
        }

        std::string newName = name;
        int index = 1;
        while (findDevice(type, newName) != -1)
        {
#if __cplusplus >= 202002L
            newName = std::format("{}-{}", name, index++);
#else
            std::stringstream ss;
            ss << name << "-" << index++;
            newName = ss.str();
#endif
        }

        // Create new device and add it to the device list
        switch (type)
        {
        case DeviceType::Camera:
        {
            LOG_F(INFO, "Trying to add a new camera instance : %s", newName.c_str());
            m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Camera>(newName));
            LOG_F(INFO, "Added new camera instance successfully");
            break;
        }
        case DeviceType::Telescope:
        {
            LOG_F(INFO, "Trying to add a new telescope instance : %s", newName.c_str());
            m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Telescope>(newName));
            LOG_F(INFO, "Added new telescope instance successfully");
            break;
        }
        case DeviceType::Focuser:
        {
            LOG_F(INFO, "Trying to add a new Focuser instance : %s", newName.c_str());
            m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Focuser>(newName));
            LOG_F(INFO, "Added new focuser instance successfully");
            break;
        }
        case DeviceType::FilterWheel:
        {
            LOG_F(INFO, "Trying to add a new filterwheel instance : %s", newName.c_str());
            m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Filterwheel>(newName));
            LOG_F(INFO, "Added new filterwheel instance successfully");
            break;
        }
        case DeviceType::Solver:
            // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Solver>(newName));
            break;
        case DeviceType::Guider:
            // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Guider>(newName));
            break;
        default:
            LOG_F(ERROR, "Invalid device type");
            break;
        }
    }

    void DeviceManager::removeDevice(DeviceType type, const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto &devices = m_devices[static_cast<int>(type)];
        for (auto it = devices.begin(); it != devices.end(); ++it)
        {
            if (*it && (*it)->getName() == name)
            {
                (*it)->disconnect();
                devices.erase(it);
                return;
            }
        }
        // spdlog::warn("Could not find device {} of type {}", name, static_cast<int>(type));
    }

    void DeviceManager::removeDevicesByName(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto &devices : m_devices)
        {
            devices.erase(std::remove_if(devices.begin(), devices.end(),
                                         [&](const std::shared_ptr<Device> &device)
                                         { return device && device->getName() == name; }),
                          devices.end());
        }
    }

    std::shared_ptr<Device> DeviceManager::getDevice(DeviceType type, const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        size_t index = findDevice(type, name);
        if (index != -1)
        {
            return m_devices[static_cast<int>(type)][index];
        }
        else
        {
            // spdlog::warn("Could not find device {} of type {}", name, static_cast<int>(type));
            return nullptr;
        }
    }

    size_t DeviceManager::findDevice(DeviceType type, const std::string &name)
    {
        auto &devices = m_devices[static_cast<int>(type)];
        for (size_t i = 0; i < devices.size(); ++i)
        {
            if (devices[i] && devices[i]->getName() == name)
            {
                return i;
            }
        }
        return -1;
    }

    std::shared_ptr<Device> DeviceManager::findDeviceByName(const std::string &name) const
    {
        for (const auto &devices : m_devices)
        {
            for (const auto &device : devices)
            {
                if (device && device->getName() == name)
                {
                    return device;
                }
            }
        }
        return nullptr;
    }

    std::shared_ptr<SimpleTask> DeviceManager::getSimpleTask(DeviceType type, const std::string &device_type, const std::string &device_name, const std::string &task_name, const nlohmann::json &params)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        LOG_F(INFO, "Trying to find %s and get %s task", device_name.c_str(), task_name.c_str());
        auto device = findDeviceByName(device_name);
        if (device != nullptr)
        {
            switch (type)
            {
            case DeviceType::Camera:
            {
                LOG_F(INFO, "Found Camera device: %s with driver: %s", device_name.c_str(), device_type.c_str());
                return std::dynamic_pointer_cast<Camera>(device)->getSimpleTask(task_name, params);
                break;
            }
            case DeviceType::Telescope:
            {
                LOG_F(INFO, "Found Telescope device: {} with driver: {}", device_name, device_type);
                return std::dynamic_pointer_cast<Telescope>(device)->getSimpleTask(task_name, params);
                break;
            }
            case DeviceType::Focuser:
            {
                LOG_F(INFO, "Found Focuser device: {} with driver: {}", device_name, device_type);
                return std::dynamic_pointer_cast<Focuser>(device)->getSimpleTask(task_name, params);
                break;
            }
            case DeviceType::FilterWheel:
            {
                LOG_F(INFO, "Found FilterWheel device: {} with driver: {}", device_name, device_type);
                return std::dynamic_pointer_cast<Filterwheel>(device)->getSimpleTask(task_name, params);
                break;
            }
            case DeviceType::Solver:
            {
                break;
            }
            case DeviceType::Guider:
            {
                break;
            }
            default:
                LOG_F(ERROR, "Invalid device type");
                break;
            }
        }
        else
        {
            LOG_F(INFO, "Device {} not found", device_name);
        }
        return nullptr;
    }
}
