/*
 * filterwheel.hpp
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

Date: 2023-6-1

Description: Basic FilterWheel Defination

*************************************************/

#pragma once

#include "device.hpp"

class Filterwheel : virtual public Device
{
public:
    Filterwheel(const std::string &name);
    ~Filterwheel();

    virtual bool connect(const IParams &params) override;

    virtual bool disconnect(const IParams &params) override;

    virtual bool reconnect(const IParams &params) override;

    virtual bool moveTo(const nlohmann::json &params);
};