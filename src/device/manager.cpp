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

#include <spdlog/spdlog.h>
#include "nlohmann/json.hpp"

namespace OpenAPT {

    DeviceManager::DeviceManager() {
        for (int i = 0; i < DeviceTypeCount; ++i) {
            m_devices[i] = std::vector<std::shared_ptr<Device>>();
        }
    }

    DeviceManager::~DeviceManager() {
        for (DeviceType type = DeviceType::Camera; type <= DeviceType::Guider; type = static_cast<DeviceType>(static_cast<int>(type) + 1)) { 
            std::vector<std::shared_ptr<Device>> devices = m_devices[static_cast<int>(type)];
            for (auto device : devices) {
                device->disconnect(); // 断开设备连接
            }
        }
    }

    std::vector<std::string> DeviceManager::getDeviceList(DeviceType type) {
                std::vector<std::string> deviceList;
                for (auto& d : m_devices[static_cast<int>(type)]) {
                    deviceList.push_back(d->getName());
                }
                return deviceList;
            }

            bool DeviceManager::addDevice(DeviceType type, const std::string& name) {
                assert(type >= DeviceType::Camera && type < DeviceType::NumDeviceTypes && "Invalid device type");
                std::string newName = name;
                int index = 1;
                while (findDevice(type, newName) != -1) {
                    newName = fmt::format("{}-{}", name, index++);
                }
                switch (type) {
                    case DeviceType::Camera:
                        m_devices[static_cast<int>(type)].push_back(std::make_shared<Camera>(newName));
                        break;
                    case DeviceType::Telescope:
                        m_devices[static_cast<int>(type)].push_back(std::make_shared<Telescope>(newName));
                        break;
                    case DeviceType::Focuser:
                        m_devices[static_cast<int>(type)].push_back(std::make_shared<Focuser>(newName));
                        break;
                    case DeviceType::FilterWheel:
                        m_devices[static_cast<int>(type)].push_back(std::make_shared<FilterWheel>(newName));
                        break;
                    case DeviceType::Solver:
                        m_devices[static_cast<int>(type)].push_back(std::make_shared<Solver>(newName));
                        break;
                    case DeviceType::Guider:
                        m_devices[static_cast<int>(type)].push_back(std::make_shared<Guider>(newName));
                        break;
                    default:
                        spdlog::error("错误的设备类型");
                        return false;
                }
                return true;
            }

            bool DeviceManager::removeDevice(DeviceType type, const std::string& name) {
                auto it = std::find_if(m_devices[static_cast<int>(type)].begin(), m_devices[static_cast<int>(type)].end(),
                        [&](std::shared_ptr<Device> device) { return device->getName() == name; });

                if (it != m_devices[static_cast<int>(type)].end()) {
                    m_devices[static_cast<int>(type)].erase(it);
                    return true;
                } else {
                    spdlog::warn("未找到指定设备 {}-{}", static_cast<int>(type), name);
                    return false;
                }
            }
            
            void DeviceManager::removeDevicesByName(const std::string& name) {
                for (int i = 0; i < static_cast<int>(DeviceTypeCount); ++i) {
                    auto it = std::remove_if(m_devices[i].begin(), m_devices[i].end(),
                                [&](std::shared_ptr<Device> device) { return device->getName() == name; });
                    m_devices[i].erase(it, m_devices[i].end());
                }
            }

            std::shared_ptr<Device> DeviceManager::getDevice(DeviceType type, const std::string& name) {
                int index = findDevice(type, name);
                if (index != -1) {
                    return m_devices[static_cast<int>(type)][index];
                } else {
                    spdlog::warn("未找到指定设备 {}-{}", static_cast<int>(type), name);
                    return nullptr;
                }
            }

            int DeviceManager::findDevice(DeviceType type, const std::string& name) {
                auto it = std::find_if(m_devices[static_cast<int>(type)].begin(), m_devices[static_cast<int>(type)].end(),
                            [&](std::shared_ptr<Device> device) { return device->getName() == name; });

                if (it != m_devices[static_cast<int>(type)].end()) {
                    return static_cast<int>(std::distance(m_devices[static_cast<int>(type)].begin(), it));
                } else {
                    return -1;
                }
            }

            std::shared_ptr<Device> DeviceManager::findDeviceByName(const std::string& name) const {
                for (auto& devList : m_devices) {
                    for (auto& device : devList) {
                        if (device->getName() == name) {
                            return device;
                        }
                    }
                }
                // 找不到则返回空指针
                return nullptr;
            }
    
}
