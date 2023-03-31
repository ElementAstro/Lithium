/*
 * basic_device.hpp
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

#pragma once

#include <string>

namespace OpenAPT {

    enum class DeviceType {
        Camera,
        Telescope,
        Focuser,
        FilterWheel,
        Solver,
        Guider,
        NumDeviceTypes
    };

    static constexpr int DeviceTypeCount = 6;

    enum class DeviceStatus {
        Unconnected,
        Connected,
        Disconnected
    };

    class Device;

    class Camera;
    class Telescope;
    class Focuser;
    class FilterWheel;
    class Guider;
    class Solver;

}


