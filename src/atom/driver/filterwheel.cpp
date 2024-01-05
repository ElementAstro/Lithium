/*
 * filterwheel.cpp
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

Description: Filterwheel Simulator and Basic Definition

**************************************************/

#include "filterwheel.hpp"

#include "atom/log/loguru.hpp"

Filterwheel::Filterwheel(const std::string &name) : Device(name)
{
    init();
}

Filterwheel::~Filterwheel()
{
}

bool Filterwheel::connect(const json &params)
{
    return true;
}

bool Filterwheel::disconnect(const json &params)
{
    return true;
}

bool Filterwheel::reconnect(const json &params)
{
    return true;
}

bool Filterwheel::isConnected()
{
    return true;
}

bool Filterwheel::moveTo(const json &params)
{
    return true;
}

bool Filterwheel::getCurrentPosition(const json &params)
{
    return true;
}