/*
 * component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-8-6

Description: Component Entry, which is used to describe the component.

**************************************************/

#pragma once

#include <string>
#include <vector>

namespace Lithium
{
    class ComponentEntry
    {
    public:
        std::string m_name;
        std::string m_main_entry;
        std::string m_component_type;
        std::string m_module_name;
        std::string m_project_name

        ComponentEntry(const std::string &name, const std::string &func_name, const std::string &component_type,
                       const std::string &module_name, const std::string &project_name)
            : m_name(name), m_main_entry(func_name), m_component_type(component_type), m_module_name(module_name), m_project_name(project_name)
        {
        }
    };

}
