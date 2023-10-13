/*
 * filterwheel.cpp
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

Description: Filterwheel Simulator and Basic Definition

**************************************************/

#include "filterwheel.hpp"

#include "loguru/loguru.hpp"

Filterwheel::Filterwheel(const std::string &name) : Device(name)
{
    DLOG_F(INFO, "Filterwheel Simulator Loaded : %s", name.c_str());
    init();
}

Filterwheel::~Filterwheel()
{
    DLOG_F(INFO, "Filterwheel Simulator Destructed");
}

bool Filterwheel::connect(const IParams &params)
{
    DLOG_F(INFO, "%s is connected", getDeviceName());
    return true;
}

bool Filterwheel::disconnect(const IParams &params)
{
    DLOG_F(INFO, "%s is disconnected", getDeviceName());
    return true;
}

bool Filterwheel::reconnect(const IParams &params)
{
    return true;
}

bool Filterwheel::moveTo(const nlohmann::json &params)
{
    return true;
}