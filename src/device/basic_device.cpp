/*
 * basic_device.cpp
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
 
Description: Basic Device Definitions
 
**************************************************/

#include <iostream>
#include <vector>
#include <string>

#include "basic_device.hpp"

namespace OpenAPT
{
    
} // namespace OpenAPT

/*
int main() {
    DeviceManager manager;
    manager.addDevice(DeviceType::Camera, "Camera1");
    manager.addDevice(DeviceType::Camera, "Camera1");
    manager.addDevice(DeviceType::Camera, "Camera2");
    manager.addDevice(DeviceType::Telescope, "Telescope1");
    manager.addDevice(DeviceType::Focuser, "EC1");
    manager.addDevice(DeviceType::FilterWheel, "FW1");
    manager.addDevice(DeviceType::Solver, "Solver1");
    manager.addDevice(DeviceType::Guider, "Guider1");

    auto cameraList = manager.getDeviceList(DeviceType::Camera);
    std::cout << "相机列表: ";
    for (auto& name : cameraList) {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    auto telescopeList = manager.getDeviceList(DeviceType::Telescope);
    std::cout << "望远镜列表: ";
    for (auto& name : telescopeList) {
        std::cout << name << " ";
    }
    std::cout << std::endl;

    spdlog::info("{}",manager.findDeviceByName("Camera2")->getName());

    auto device1 = manager.findDeviceByName("Camera1");
    if (device1 != nullptr) {
        spdlog::info("{}",device1->connect());
        // 找到了设备
        // ...
    } else {
        spdlog::error("找不到设备 Camera1");
    }

    auto deviceaa = manager.findDeviceByName("Camera1aaa");
    if (deviceaa != nullptr) {
        spdlog::info("{}",deviceaa->connect());
        // 找到了设备
        // ...
    } else {
        spdlog::error("找不到设备 Camera1");
    }

    //manager.removeDevice(DeviceType::Camera,"Camera1");
    manager.removeDevicesByName("Camera1");
    auto device = manager.findDeviceByName("Camera1");
    if (device != nullptr) {
        spdlog::info("{}",device->connect());
        // 找到了设备
        // ...
    } else {
        spdlog::error("找不到设备 Camera1");
    }

    return 0;
}
*/

