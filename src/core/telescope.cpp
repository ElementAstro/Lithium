/*
 * telescope.cpp
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

Description: Telescope Simulator and Basic Definition

**************************************************/

#include "telescope.hpp"

#include "loguru/loguru.hpp"

Telescope::Telescope(const std::string &name) : Device(name)
{
    init();
}

Telescope::~Telescope()
{
}

bool Telescope::connect(const json &params)
{
    return true;
}

bool Telescope::disconnect(const json &params)
{
    return true;
}

bool Telescope::reconnect(const json &params)
{
    return true;
}

bool Telescope::isConnected()
{
    return true;
}

bool Telescope::SlewTo(const json &params)
{
    return true;
}

bool Telescope::Abort(const json &params)
{
    return true;
}

bool Telescope::isSlewing(const json &params)
{
    return true;
}

std::string Telescope::getCurrentRA(const json &params)
{
    return "";
}

std::string Telescope::getCurrentDec(const json &params)
{
    return "";
}

bool Telescope::StartTracking(const json &params)
{
    return true;
}

bool Telescope::StopTracking(const json &params)
{
    return true;
}

bool Telescope::setTrackingMode(const json &params)
{
    return true;
}

bool Telescope::setTrackingSpeed(const json &params)
{
    return true;
}

std::string Telescope::getTrackingMode(const json &params)
{
    return "";
}

std::string Telescope::getTrackingSpeed(const json &params)
{
    return "";
}

bool Telescope::Home(const json &params)
{
    return true;
}

bool Telescope::isAtHome(const json &params)
{
    return true;
}

bool Telescope::setHomePosition(const json &params)
{
    return true;
}

bool Telescope::isHomeAvailable(const json &params)
{
    return true;
}

bool Telescope::Park(const json &params)
{
    return true;
}

bool Telescope::Unpark(const json &params)
{
    return true;
}

bool Telescope::isAtPark(const json &params)
{
    return true;
}

bool Telescope::setParkPosition(const json &params)
{
    return true;
}

bool Telescope::isParkAvailable(const json &params)
{
    return true;
}
