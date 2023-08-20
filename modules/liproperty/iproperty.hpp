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

struct IPropertyBase
{
    IPropertyBase();
    std::string device_name;
    std::string device_uuid;
    std::string message_uuid;
    std::string name;
    bool need_check;
    PossibleValueType pv_type;

    std::string get_func;
    std::string set_func;
};

struct INumberProperty : public IPropertyBase
{
    INumberProperty();
    double value;
    std::vector<double> possible_values;
};

struct IStringProperty : public IPropertyBase
{
    IStringProperty();
    std::string value;
    std::vector<std::string> possible_values;
};

struct IBoolProperty : public IPropertyBase
{
    IBoolProperty();
    bool value;
    std::vector<bool> possible_values;
};

struct INumberVector : public IPropertyBase
{
    INumberVector();
    std::vector<double> value;
    std::vector<std::vector<double>> possible_values;
};
