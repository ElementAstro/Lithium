/*
 * focuser.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
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

Date: 2023-3-29

Description: Focuser Simulator and Basic Definition

**************************************************/

#include "focuser.hpp"

#include "atom/log/loguru.hpp"

Focuser::Focuser(const std::string &name) : Device(name)
{
    DLOG_F(INFO, "Focuser Simulator Loaded : %s", name.c_str());
    init();
}

Focuser::~Focuser()
{
    DLOG_F(INFO, "Focuser Simulator Destructed");
}

bool Focuser::connect(const json &params)
{
    DLOG_F(INFO, "%s is connected", getDeviceName());
    return true;
}

bool Focuser::disconnect(const json &params)
{
    DLOG_F(INFO, "%s is disconnected", getDeviceName());
    return true;
}

bool Focuser::reconnect(const json &params)
{
    return true;
}

bool Focuser::isConnected()
{
    return true;
}

bool Focuser::moveTo(const json &params)
{
    return true;
}

bool Focuser::moveToAbsolute(const json &params)
{
    return true;
}

bool Focuser::moveStep(const json &params)
{
    return true;
}

bool Focuser::moveStepAbsolute(const json &params)
{
    return true;
}

bool Focuser::AbortMove(const json &params)
{
    return true;
}

int Focuser::getMaxPosition(const json &params)
{
    return 0;
}

bool Focuser::setMaxPosition(const json &params)
{
    return true;
}

bool Focuser::isGetTemperatureAvailable(const json &params)
{
    return true;
}

double Focuser::getTemperature(const json &params)
{
    return 0.0;
}

bool Focuser::isAbsoluteMoveAvailable(const json &params)
{
    return true;
}

bool Focuser::isManualMoveAvailable(const json &params)
{
    return true;
}

int Focuser::getCurrentPosition(const json &params)
{
    return 0;
}

bool Focuser::haveBacklash(const json &params)
{
    return true;
}

bool Focuser::setBacklash(const json &params)
{
    return true;
}