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

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <string_view>

// Helper to get the number of enum entries using constexpr reflection
template <typename Enum>
constexpr auto enumSize() {
    if constexpr (requires { Enum::LastEnumValue; }) {
        return static_cast<std::size_t>(Enum::LastEnumValue);
    } else {
        return 0;  // Handle the error or throw a static_assert
    }
}

template <typename Enum, size_t N>
struct EnumReflection {
    std::array<std::pair<Enum, std::string_view>, N> data;

    constexpr EnumReflection(const std::pair<Enum, std::string_view> (&arr)[N])
        : data{} {
        std::copy(std::begin(arr), std::end(arr), std::begin(data));
    }

    constexpr std::string_view toString(Enum e) const {
        auto it = std::find_if(data.begin(), data.end(), [e](const auto& pair) {
            return pair.first == e;
        });
        if (it != data.end()) {
            return it->second;
        }
        return "Undefined";
    }

    constexpr std::optional<Enum> fromString(std::string_view str) const {
        auto it = std::find_if(
            data.begin(), data.end(),
            [str](const auto& pair) { return pair.second == str; });
        if (it != data.end()) {
            return it->first;
        }
        return std::nullopt;
    }
};

enum class ComponentType {
    NONE,
    SHREAD,
    SHREAD_INJECTED,
    Script,
    Executable,
    Task,
    LastEnumValue
};

constexpr auto componentTypeReflection =
    EnumReflection<ComponentType, enumSize<ComponentType>()>(
        {{ComponentType::NONE, "none"},
         {ComponentType::SHREAD, "shared"},
         {ComponentType::SHREAD_INJECTED, "injected"},
         {ComponentType::Script, "script"},
         {ComponentType::Executable, "executable"},
         {ComponentType::Task, "task"}});

#endif
