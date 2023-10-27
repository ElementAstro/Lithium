/*
 * focuser.cpp
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

Description: Focuser Simulator and Basic Definition

**************************************************/

#include "focuser.hpp"

#include "loguru/loguru.hpp"

Focuser::Focuser(const std::string &name) : Device(name)
{
    DLOG_F(INFO, "Focuser Simulator Loaded : %s", name.c_str());
    init();
}

Focuser::~Focuser()
{
    DLOG_F(INFO, "Focuser Simulator Destructed");
}

bool Focuser::connect(const nlohmann::json &params)
{
    DLOG_F(INFO, "%s is connected", getDeviceName());
    return true;
}

bool Focuser::disconnect(const nlohmann::json &params)
{
    DLOG_F(INFO, "%s is disconnected", getDeviceName());
    return true;
}

bool Focuser::reconnect(const nlohmann::json &params)
{
    return true;
}

bool Focuser::moveTo(const nlohmann::json &params)
{
    return true;
}

bool Focuser::moveToAbsolute(const nlohmann::json &params)
{
    return true;
}

bool Focuser::moveStep(const nlohmann::json &params)
{
    return true;
}

bool Focuser::moveStepAbsolute(const nlohmann::json &params)
{
    return true;
}

bool Focuser::AbortMove(const nlohmann::json &params)
{
    return true;
}

int Focuser::getMaxPosition(const nlohmann::json &params)
{
    return 0;
}

bool Focuser::setMaxPosition(const nlohmann::json &params)
{
    return true;
}

bool Focuser::isGetTemperatureAvailable(const nlohmann::json &params)
{
    return true;
}

double Focuser::getTemperature(const nlohmann::json &params)
{
    return 0.0;
}

bool Focuser::isAbsoluteMoveAvailable(const nlohmann::json &params)
{
    return true;
}

bool Focuser::isManualMoveAvailable(const nlohmann::json &params)
{
    return true;
}

int Focuser::getCurrentPosition(const nlohmann::json &params)
{
    return 0;
}

bool Focuser::haveBacklash(const nlohmann::json &params)
{
    return true;
}

bool Focuser::setBacklash(const nlohmann::json &params)
{
    return true;
}