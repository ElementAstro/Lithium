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

#include "atom/function/enum.hpp"

enum class ComponentType {
    NONE,
    SHARED,
    SHARED_INJECTED,
    SCRIPT,
    EXECUTABLE,
    TASK,
    LAST_ENUM_VALUE
};

template <>
struct EnumTraits<ComponentType> {
    static constexpr std::array<ComponentType, 7> VALUES = {
        ComponentType::NONE,
        ComponentType::SHARED,
        ComponentType::SHARED_INJECTED,
        ComponentType::SCRIPT,
        ComponentType::EXECUTABLE,
        ComponentType::TASK,
        ComponentType::LAST_ENUM_VALUE};

    static constexpr std::array<std::string_view, 7> NAMES = {
        "NONE",       "SHARED", "SHARED_INJECTED", "SCRIPT",
        "EXECUTABLE", "TASK",   "LAST_ENUM_VALUE"};
};

#endif  // ATOM_COMPONENT_TYPES_HPP
