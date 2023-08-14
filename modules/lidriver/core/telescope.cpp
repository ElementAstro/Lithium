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
    LOG_F(INFO, "Telescope Simulator Loaded : %s", name.c_str());
    init();
}

Telescope::~Telescope()
{
    LOG_F(INFO, "Telescope Simulator Destructed");
}

bool Telescope::connect(const std::string &name)
{
    LOG_F(INFO, "%s is connected", name.c_str());
    return true;
}

bool Telescope::disconnect()
{
    LOG_F(INFO, "%s is disconnected", getStringProperty("name")->value.c_str());
    return true;
}

bool Telescope::reconnect()
{
    return true;
}

bool Telescope::SlewTo(const nlohmann::json &params)
{
    return true;
}

bool Telescope::Abort(const nlohmann::json &params)
{
    return true;
}

bool Telescope::isSlewing(const nlohmann::json &params)
{
    return true;
}

std::string Telescope::getCurrentRA(const nlohmann::json &params)
{
    return "";
}

std::string Telescope::getCurrentDec(const nlohmann::json &params)
{
    return "";
}

bool Telescope::StartTracking(const nlohmann::json &params)
{
    return true;
}

bool Telescope::StopTracking(const nlohmann::json &params)
{
    return true;
}

bool Telescope::setTrackingMode(const nlohmann::json &params)
{
    return true;
}

bool Telescope::setTrackingSpeed(const nlohmann::json &params)
{
    return true;
}

std::string Telescope::getTrackingMode(const nlohmann::json &params)
{
    return "";
}

std::string Telescope::getTrackingSpeed(const nlohmann::json &params)
{
    return "";
}

bool Telescope::Home(const nlohmann::json &params)
{
    return true;
}

bool Telescope::isAtHome(const nlohmann::json &params)
{
    return true;
}

bool Telescope::setHomePosition(const nlohmann::json &params)
{
    return true;
}

bool Telescope::isHomeAvailable(const nlohmann::json &params)
{
    return true;
}

bool Telescope::Park(const nlohmann::json &params)
{
    return true;
}

bool Telescope::Unpark(const nlohmann::json &params)
{
    return true;
}

bool Telescope::isAtPark(const nlohmann::json &params)
{
    return true;
}

bool Telescope::setParkPosition(const nlohmann::json &params)
{
    return true;
}

bool Telescope::isParkAvailable(const nlohmann::json &params)
{
    return true;
}
