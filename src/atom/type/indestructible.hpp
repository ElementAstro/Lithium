/*
 * indestructible.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-16

Description: Indestructible

**************************************************/

#ifndef ATOM_TYPE_INDESTRUCTIBLE_HPP
#define ATOM_TYPE_INDESTRUCTIBLE_HPP

#include <type_traits>
#include <utility>

template <typename T>
struct Indestructible {
    union {
        T object;
        char dummy;
    };

    template <typename... Args>
        requires std::is_constructible_v<T, Args...>
    constexpr explicit Indestructible(std::in_place_t, Args&&... args)
        : object(std::forward<Args>(args)...) {}

    ~Indestructible() {
        object.~T();
    }

    constexpr Indestructible(const Indestructible& other)
        requires std::is_trivially_copy_constructible_v<T>
    = default;

    constexpr Indestructible(const Indestructible& other)
        requires(!std::is_trivially_copy_constructible_v<T>)
        : object(other.object) {}

    constexpr Indestructible(Indestructible&& other)
        requires std::is_trivially_move_constructible_v<T>
    = default;

    constexpr Indestructible(Indestructible&& other)
        requires(!std::is_trivially_move_constructible_v<T>)
        : object(std::move(other.object)) {}

    constexpr Indestructible& operator=(const Indestructible& other)
        requires std::is_trivially_copy_assignable_v<T>
    = default;

    constexpr Indestructible& operator=(const Indestructible& other)
        requires(!std::is_trivially_copy_assignable_v<T>)
    {
        object = other.object;
        return *this;
    }

    constexpr Indestructible& operator=(Indestructible&& other)
        requires std::is_trivially_move_assignable_v<T>
    = default;

    constexpr Indestructible& operator=(Indestructible&& other)
        requires(!std::is_trivially_move_assignable_v<T>)
    {
        object = std::move(other.object);
        return *this;
    }

    constexpr T& get() & { return object; }
    constexpr const T& get() const& { return object; }
    constexpr T&& get() && { return std::move(object); }
    constexpr const T&& get() const&& { return std::move(object); }
};

#endif