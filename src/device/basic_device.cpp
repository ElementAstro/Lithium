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
    Device::Device(const std::string &name)
    {
        _name = name;
    }

    Device::~Device() {}

    Camera::Camera(const std::string &name) : Device(name) {}

    Camera::~Camera() {}

    Telescope::Telescope(const std::string &name) : Device(name) {}

    Telescope::~Telescope() {}

    Focuser::Focuser(const std::string &name) : Device(name) {}

    Focuser::~Focuser() {}

    Filterwheel::Filterwheel(const std::string &name) : Device(name) {}

    Filterwheel::~Filterwheel() {}

    Solver::Solver(const std::string &name) : Device(name) {}

    Solver::~Solver() {}
} // namespace OpenAPT
