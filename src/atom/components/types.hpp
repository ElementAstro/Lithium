/*
 * types.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-3-1

Description: Basic Component Types Definition and Some Utilities

**************************************************/

#ifndef ATOM_COMPONENT_TYPES_HPP
#define ATOM_COMPONENT_TYPES_HPP

#define ATOM_COMPONENT_TYPE_COUNT

#include <string>

enum class ComponentType {
    NONE,
    SHREAD,
    ALONE,
    SHREAD_INJECTED,
    Script,
    Executable,
    Task,
    MaxType
};

[[maybe_unused]] [[nodiscard]] std::string toString(ComponentType type);

[[maybe_unused]] std::ostream &operator<<(std::ostream &os, ComponentType type);

[[maybe_unused]] std::istream &operator>>(std::istream &is,
                                          ComponentType &type);

[[maybe_unused]] [[nodiscard]] ComponentType toComponentType(const int &type);

#endif
