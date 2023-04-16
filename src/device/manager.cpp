/*
 * manager.cpp
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

#include "manager.hpp"
#include "basic_device.hpp"

#include "indi/indicamera.hpp"
#include "indi/indifocuser.hpp"
#include "indi/inditelescope.hpp"

#include <spdlog/spdlog.h>
#include "nlohmann/json.hpp"

namespace OpenAPT
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

        // 检查设备类型是否合法
        if (type < DeviceType::Camera || type > DeviceType::Guider)
        {
            throw std::invalid_argument("Invalid device type");
        }

        // 检查设备名称是否重复
        if (findDeviceByName(name))
        {
            spdlog::warn("A device with name {} already exists, please choose a different name", name);
            return;
        }

        std::string newName = name;
        int index = 1;
        while (findDevice(type, newName) != -1)
        {
            newName = fmt::format("{}-{}", name, index++);
        }

        // 创建新设备并添加到设备列表
        switch (type)
        {
        case DeviceType::Camera:
        {
            spdlog::debug("Trying to add a new camera instance : {}", newName);
            m_devices[static_cast<int>(type)].emplace_back(std::make_shared<INDICamera>(newName));
            spdlog::debug("Added new camera instance successfully");
            break;
        }
        case DeviceType::Telescope:
            // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Telescope>(newName));
            break;
        case DeviceType::Focuser:
        {
            spdlog::debug("Trying to add a new Focuser instance : {}", newName);
            m_devices[static_cast<int>(type)].emplace_back(std::make_shared<INDIFocuser>(newName));
            spdlog::debug("Added new Focuser instance successfully");
            break;
        }
        case DeviceType::FilterWheel:
            // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<FilterWheel>(newName));
            break;
        case DeviceType::Solver:
            // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Solver>(newName));
            break;
        case DeviceType::Guider:
            // m_devices[static_cast<int>(type)].emplace_back(std::make_shared<Guider>(newName));
            break;
        default:
            spdlog::error("Invalid device type");
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
        spdlog::warn("Could not find device {} of type {}", name, static_cast<int>(type));
    }

    void DeviceManager::removeDevicesByName(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        for (auto &devices : m_devices)
        {
            devices.erase(std::remove_if(devices.begin(), devices.end(),
                                         [&](std::shared_ptr<Device> device)
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
            spdlog::warn("Could not find device {} of type {}", name, static_cast<int>(type));
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

    std::shared_ptr<Camera> DeviceManager::getCamera(const std::string &name)
    {
        std::lock_guard<std::mutex> lock(m_mutex);

        size_t index = findDevice(DeviceType::Camera, name);
        if (index != -1)
        {
            return std::dynamic_pointer_cast<Camera>(m_devices[static_cast<int>(DeviceType::Camera)][index]);
        }
        else
        {
            spdlog::warn("Could not find camera {}", name);
            return nullptr;
        }
    }

    std::shared_ptr<SimpleTask> DeviceManager::getSimpleTask(DeviceType type, const std::string &device_type, const std::string &device_name, const std::string &task_name, const nlohmann::json &params)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        spdlog::debug("Trying to find {} and get {} task", device_name, task_name);
        auto device = m_DeviceManager.findDeviceByName(device_name);
        if (device != nullptr)
        {
            switch (type)
            {
            case DeviceType::Camera:
            {
                if (device_type == "INDI")
                {
                    spdlog::debug("Found Camera device: {} with driver: {}", device_name, device_type);
                    return std::dynamic_pointer_cast<OpenAPT::INDICamera>(device)->getSimpleTask(task_name,params);
                }
                else if (device_type == "ASCOM")
                {
                    spdlog::debug("Found Camera device: {} with driver: {}", device_name, device_type);
                }
                break;
            }
            case DeviceType::Telescope:
            {
                if (device_type == "INDI")
                {
                    spdlog::debug("Found Telescope device: {} with driver: {}", device_name, device_type);
                    //return std::dynamic_pointer_cast<OpenAPT::INDITelescope>(device)->SlewTask();
                }
                break;
            }
            case DeviceType::Focuser:
            {
                if (device_type == "INDI")
                {
                    spdlog::debug("Found Focuser device: {} with driver: {}", device_name, device_type);
                    return std::dynamic_pointer_cast<OpenAPT::INDIFocuser>(device)->getSimpleTask(task_name,params);
                }
                break;
            }
            case DeviceType::FilterWheel:
            {
                if (device_type == "INDI")
                {
                    spdlog::debug("Found FilterWheel device: {} with driver: {}", device_name, device_type);
                    //return std::dynamic_pointer_cast<OpenAPT::INDIFilterWheel>(device)->SetFilterTask();
                }
                break;
            }
            case DeviceType::Solver:
            {
                if (device_type == "ASTAP")
                {
                    spdlog::debug("Found Solver: ASTAP");
                    //return std::make_shared<OpenAPT::ASTapSolverTask>();
                }
                if (device_type == "Astrometry")
                {
                    spdlog::debug("Found Solver: Astrometry");
                    //return std::make_shared<OpenAPT::AstrometrySolverTask>();
                }
                break;
            }
            case DeviceType::Guider:
            {
                if (device_type == "PHD2")
                {
                    spdlog::debug("Found Guider device: {} with driver: {}", device_name, device_type);
                    //return std::make_shared<OpenAPT::PHD2GuiderTask>();
                }
                break;
            }
            default:
                spdlog::error("Invalid device type");
                break;
            }
        }
        else
        {
            spdlog::debug("Device {} not found", device_name);
        }
        return nullptr;
    }

    std::shared_ptr<ConditionalTask> DeviceManager::getConditionalTask(DeviceType type, const std::string &device_type, const std::string &device_name, const std::string &task_name, const nlohmann::json &params)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        spdlog::debug("Trying to find {} and get {} task", device_name, task_name);
        auto device = m_DeviceManager.findDeviceByName(device_name);
        if (device!= nullptr)
        {
            switch (type)
            {
                case DeviceType::Camera: {

                }
                case DeviceType::Telescope: {

                }
                case DeviceType::Focuser : {

                }
                case DeviceType::FilterWheel: {

                }
                case DeviceType::Solver: {

                }
                case DeviceType::Guider: {

                }
                default: 
                    spdlog::error("Invalid device type");
                    break;
            }
        }
        return nullptr;
    }

    std::shared_ptr<LoopTask> DeviceManager::getLoopTask(DeviceType type, const std::string &device_type, const std::string &device_name, const std::string &task_name, const nlohmann::json &params)
    {
        return nullptr;
    }

}
