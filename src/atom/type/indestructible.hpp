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

/**
 * @brief A class template for creating an object that cannot be destructed.
 *
 * @tparam T The type of the object to create.
 */
template <typename T>
struct Indestructible {
    union {
        T object;    ///< The object being stored.
        char dummy;  ///< Dummy character used for alignment purposes.
    };

    /**
     * @brief Constructs an Indestructible object with the provided arguments.
     *
     * @tparam Args The types of the arguments to construct the object with.
     * @param args The arguments to construct the object with.
     */
    template <typename... Args>
        requires std::is_constructible_v<T, Args...>
    constexpr explicit Indestructible(std::in_place_t /*unused*/,
                                      Args&&... args)
        : object(std::forward<Args>(args)...) {}

    /**
     * @brief Destructor.
     *
     * If T is not trivially destructible, the destructor explicitly calls the
     * destructor of the stored object.
     */
    ~Indestructible() {
        if constexpr (!std::is_trivially_destructible_v<T>) {
            object.~T();
        }
    }

    /**
     * @brief Copy constructor.
     *
     * If T is trivially copy constructible, the copy constructor is defaulted.
     * Otherwise, the copy constructor copies the stored object from another
     * Indestructible object.
     *
     * @param other The Indestructible object to copy from.
     */
    constexpr Indestructible(const Indestructible& other)
        requires std::is_trivially_copy_constructible_v<T>
    = default;

    /**
     * @brief Copy constructor.
     *
     * If T is not trivially copy constructible, the copy constructor copies the
     * stored object from another Indestructible object.
     *
     * @param other The Indestructible object to copy from.
     */
    constexpr Indestructible(const Indestructible& other)
        requires(!std::is_trivially_copy_constructible_v<T>)
        : object(other.object) {}

    /**
     * @brief Move constructor.
     *
     * If T is trivially move constructible, the move constructor is defaulted.
     * Otherwise, the move constructor moves the stored object from another
     * Indestructible object.
     *
     * @param other The Indestructible object to move from.
     */
    constexpr Indestructible(Indestructible&& other)
        requires std::is_trivially_move_constructible_v<T>
    = default;

    /**
     * @brief Move constructor.
     *
     * If T is not trivially move constructible, the move constructor moves the
     * stored object from another Indestructible object.
     *
     * @param other The Indestructible object to move from.
     */
    constexpr Indestructible(Indestructible&& other) noexcept
        requires(!std::is_trivially_move_constructible_v<T>)
        : object(std::move(other.object)) {}

    /**
     * @brief Copy assignment operator.
     *
     * If T is trivially copy assignable, the copy assignment operator is
     * defaulted. Otherwise, the copy assignment operator copies the stored
     * object from another Indestructible object.
     *
     * @param other The Indestructible object to copy assign from.
     * @return Reference to the assigned Indestructible object.
     */
    constexpr Indestructible& operator=(const Indestructible& other)
        requires std::is_trivially_copy_assignable_v<T>
    = default;

    /**
     * @brief Copy assignment operator.
     *
     * If T is not trivially copy assignable, the copy assignment operator
     * copies the stored object from another Indestructible object.
     *
     * @param other The Indestructible object to copy assign from.
     * @return Reference to the assigned Indestructible object.
     */
    constexpr Indestructible& operator=(const Indestructible& other)
        requires(!std::is_trivially_copy_assignable_v<T>)
    {
        if (this != &other) {
            object = other.object;
        }
        return *this;
    }

    /**
     * @brief Move assignment operator.
     *
     * If T is trivially move assignable, the move assignment operator is
     * defaulted. Otherwise, the move assignment operator moves the stored
     * object from another Indestructible object.
     *
     * @param other The Indestructible object to move assign from.
     * @return Reference to the assigned Indestructible object.
     */
    constexpr Indestructible& operator=(Indestructible&& other)
        requires std::is_trivially_move_assignable_v<T>
    = default;

    /**
     * @brief Move assignment operator.
     *
     * If T is not trivially move assignable, the move assignment operator moves
     * the stored object from another Indestructible object.
     *
     * @param other The Indestructible object to move assign from.
     * @return Reference to the assigned Indestructible object.
     */
    constexpr auto operator=(Indestructible&& other) noexcept -> Indestructible&
        requires(!std::is_trivially_move_assignable_v<T>)
    {
        if (this != &other) {
            object = std::move(other.object);
        }
        return *this;
    }

    /**
     * @brief Returns a reference to the stored object.
     *
     * @return Reference to the stored object.
     */
    constexpr T& get() & { return object; }

    /**
     * @brief Returns a const reference to the stored object.
     *
     * @return Const reference to the stored object.
     */
    constexpr const T& get() const& { return object; }

    /**
     * @brief Returns an rvalue reference to the stored object.
     *
     * @return Rvalue reference to the stored object.
     */
    constexpr T&& get() && { return std::move(object); }

    /**
     * @brief Returns a const rvalue reference to the stored object.
     *
     * @return Const rvalue reference to the stored object.
     */
    constexpr const T&& get() const&& { return std::move(object); }
};

#endif  // ATOM_TYPE_INDESTRUCTIBLE_HPP
