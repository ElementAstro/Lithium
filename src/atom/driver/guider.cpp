/*
 * guider.cpp
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

Description: Guider Simulator and Basic Definition

**************************************************/

#include "guider.hpp"

#include "atom/log/loguru.hpp"

Guider::Guider(const std::string &name) : Device(name)
{
    DLOG_F(INFO, "Guider Simulator Loaded : %s", name.c_str());
    init();
}

Guider::~Guider()
{
    DLOG_F(INFO, "Guider Simulator Destructed");
}

bool Guider::connect(const nlohmann::json &params)
{
    DLOG_F(INFO, "%s is connected", getDeviceName());
    return true;
}

bool Guider::disconnect(const nlohmann::json &params)
{
    DLOG_F(INFO, "%s is disconnected", getDeviceName());
    return true;
}

bool Guider::reconnect(const nlohmann::json &params)
{
    return true;
}