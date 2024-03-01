/*
 * types.cpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: Basic Component Types Definition and Some Utilities

**************************************************/

#include "types.hpp"

std::string toString(ComponentType type)
{
    switch (type)
    {
    case ComponentType::NONE:
        return "NONE";
    case ComponentType::SHREAD:
        return "SHREAD";
    case ComponentType::ALONE:
        return "ALONE";
    case ComponentType::SHREAD_INJECTED:
        return "SHREAD_INJECTED";
    case ComponentType::Script:
        return "Script";
    case ComponentType::Executable:
        return "Executable";
    case ComponentType::Task:
        return "Task";
    default:
        return "Unknown";
    }
}

std::ostream &operator<<(std::ostream &os, ComponentType type)
{
    os << toString(type);
    return os;
}

std::istream &operator>>(std::istream &is, ComponentType &type)
{
    std::string str;
    is >> str;
    if (str == "NONE")
    {
        type = ComponentType::NONE;
    }
    else if (str == "SHREAD")
    {
        type = ComponentType::SHREAD;
    }
    else if (str == "ALONE")
    {
        type = ComponentType::ALONE;
    }
    else if (str == "SHREAD_INJECTED")
    {
        type = ComponentType::SHREAD_INJECTED;
    }
    else if (str == "Script")
    {
        type = ComponentType::Script;
    }
    else if (str == "Executable")
    {
        type = ComponentType::Executable;
    }
    else if (str == "Task")
    {
        type = ComponentType::Task;
    }
    else
    {
        type = ComponentType::NONE;
    }
    return is;
}

ComponentType toComponentType(const int &type)
{
    if (type == 0)
    {
        return ComponentType::NONE;
    }
    else if (type == 1)
    {
        return ComponentType::SHREAD;
    }
    else if (type == 2)
    {
        return ComponentType::ALONE;
    }
    else if (type == 3)
    {
        return ComponentType::SHREAD_INJECTED;
    }
    else if (type == 4)
    {
        return ComponentType::Script;
    }
    else if (type == 5)
    {
        return ComponentType::Executable;
    }
    else if (type == 6)
    {
        return ComponentType::Task;
    }
    return ComponentType::NONE;
}