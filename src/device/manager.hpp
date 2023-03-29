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
#include <functional>

namespace OpenAPT {

    class DeviceManager {
        public:
            DeviceManager();
            ~DeviceManager();

            std::vector<std::string> getDeviceList(DeviceType type);
            bool addDevice(DeviceType type, const std::string& name);
            bool removeDevice(DeviceType type, const std::string& name);
            void removeDevicesByName(const std::string& name);
            std::shared_ptr<Device> getDevice(DeviceType type, const std::string& name);
            int findDevice(DeviceType type, const std::string& name);
            std::shared_ptr<Device> findDeviceByName(const std::string& name) const;

        private:
            std::vector<std::shared_ptr<Device>> m_devices[6];
    };

}