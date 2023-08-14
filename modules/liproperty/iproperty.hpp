/*
 * iproperty.hpp
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

Description: Property type definition

**************************************************/

#pragma once

#include <string>
#include <vector>

enum class PossibleValueType
{
    None,
    Range,
    Value
};

struct INumberProperty
{
    INumberProperty();
    std::string device_name;
    std::string device_uuid;
    std::string message_uuid;
    std::string name;
    std::vector<double> possible_values;
    bool need_check;
    PossibleValueType pv_type;
    double value;
};

struct IStringProperty
{
    IStringProperty();
    std::string device_name;
    std::string device_uuid;
    std::string message_uuid;
    std::string name;
    std::vector<std::string> possible_values;
    bool need_check;
    PossibleValueType pv_type;
    std::string value;
};

struct IBoolProperty
{
    IBoolProperty();
    std::string device_name;
    std::string device_uuid;
    std::string message_uuid;
    std::string name;
    std::vector<bool> possible_values;
    bool need_check;
    PossibleValueType pv_type;
    bool value;
};

struct INumberVector
{
    INumberVector();
    std::string device_name;
    std::string device_uuid;
    std::string message_uuid;
    std::string name;
    std::vector<std::vector<double>> possible_values;
    bool need_check;
    PossibleValueType pv_type;
    std::vector<double> value;
};