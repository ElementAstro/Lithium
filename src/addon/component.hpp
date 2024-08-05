/*
 * component.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2023-8-6

Description: Component Entry, which is used to describe the component.

**************************************************/

#ifndef LITIHUM_ADDON_COMPONENT_HPP
#define LITIHUM_ADDON_COMPONENT_HPP

#include <string>
#include <vector>

namespace lithium {
struct ComponentEntry {
    std::string name;
    std::string func_name;
    std::string component_type;
    std::string module_name;
    std::vector<std::string> dependencies;

    ComponentEntry(std::string name, std::string func_name,
                   std::string component_type, std::string module_name)
        : name(std::move(name)),
          func_name(std::move(func_name)),
          component_type(std::move(component_type)),
          module_name(std::move(module_name)) {}
};

}  // namespace lithium

#endif
