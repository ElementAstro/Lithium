/*
 * iproperty.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-3-29

Description: Property type definition

**************************************************/

#pragma once

#include <string>
#include <vector>
#include <unordered_map>

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
