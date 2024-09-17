/*
 * no_offset_ptr.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-3

Description: No Offset Pointer

**************************************************/

#ifndef ATOM_TYPE_NO_OFFSET_PTR_HPP
#define ATOM_TYPE_NO_OFFSET_PTR_HPP

#include <cassert>
#include <concepts>
#include <type_traits>
#include <utility>

/**
 * @brief A lightweight pointer-like class that manages an object of type T
 * without dynamic memory allocation.
 *
 * @tparam T The type of the object to manage.
 */
template <typename T>
    requires std::is_object_v<T>
class UnshiftedPtr {
public:
    /**
     * @brief Default constructor. Constructs the managed object using T's
     * default constructor.
     *
     * @note This constructor is noexcept if T's default constructor is
     * noexcept.
     */
    constexpr UnshiftedPtr() noexcept(
        std::is_nothrow_default_constructible_v<T>) {
        new (&storage_) T;
    }

    /**
     * @brief Constructor that initializes the managed object with given
     * arguments.
     *
     * @tparam Args Parameter pack for constructor arguments.
     * @param args Arguments to forward to the constructor of T.
     *
     * @note This constructor is noexcept if T's constructor with the given
     * arguments is noexcept.
     */
    template <typename... Args>
        requires std::constructible_from<T, Args...>
    constexpr explicit UnshiftedPtr(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
        new (&storage_) T(std::forward<Args>(args)...);
    }

    /**
     * @brief Destructor. Destroys the managed object.
     */
    constexpr ~UnshiftedPtr() noexcept { get().~T(); }

    // Disable copying and moving if T is non-copyable or non-movable
    UnshiftedPtr(const UnshiftedPtr&) noexcept(
        std::is_nothrow_copy_constructible_v<T>) = delete;
    UnshiftedPtr(UnshiftedPtr&&) noexcept(
        std::is_nothrow_move_constructible_v<T>) = delete;

    auto operator=(const UnshiftedPtr&) noexcept(
        std::is_nothrow_copy_assignable_v<T>) -> UnshiftedPtr& = delete;
    auto operator=(UnshiftedPtr&&) noexcept(
        std::is_nothrow_move_assignable_v<T>) -> UnshiftedPtr& = delete;

    /**
     * @brief Provides pointer-like access to the managed object.
     *
     * @return A pointer to the managed object.
     */
    constexpr auto operator->() noexcept -> T* { return &get(); }

    /**
     * @brief Provides const pointer-like access to the managed object.
     *
     * @return A const pointer to the managed object.
     */
    constexpr auto operator->() const noexcept -> const T* { return &get(); }

    /**
     * @brief Dereferences the managed object.
     *
     * @return A reference to the managed object.
     */
    constexpr auto operator*() noexcept -> T& { return get(); }

    /**
     * @brief Dereferences the managed object.
     *
     * @return A const reference to the managed object.
     */
    constexpr auto operator*() const noexcept -> const T& { return get(); }

    /**
     * @brief Resets the managed object by calling its destructor and
     * reconstructing it in-place.
     *
     * @tparam Args Parameter pack for constructor arguments.
     * @param args Arguments to forward to the constructor of T.
     *
     * @note This function is noexcept if T's destructor and constructor with
     * the given arguments are noexcept.
     */
    template <typename... Args>
        requires std::constructible_from<T, Args...>
    constexpr void reset(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...> &&
        std::is_nothrow_destructible_v<T>) {
        get().~T();
        new (&storage_) T(std::forward<Args>(args)...);
    }

    /**
     * @brief Emplaces a new object in place with the provided arguments.
     *
     * @tparam Args The types of the arguments to construct the object with.
     * @param args The arguments to construct the object with.
     */
    template <typename... Args>
        requires std::constructible_from<T, Args...>
    constexpr void emplace(Args&&... args) noexcept(
        std::is_nothrow_constructible_v<T, Args...>) {
        reset(std::forward<Args>(args)...);
    }

    /**
     * @brief Releases ownership of the managed object without destroying it.
     *        This effectively "releases" the managed object to be used
     * elsewhere. The UnshiftedPtr should not be used after calling this unless
     * reset.
     *
     * @return A pointer to the managed object.
     */
    [[nodiscard]] constexpr auto release() noexcept -> T* {
        T* ptr = new (&storage_) T(std::move(get()));
        get().~T();  // No managed object left in storage
        return ptr;
    }

    /**
     * @brief Checks if the managed object has a value.
     *
     * @return True if the managed object has a value, false otherwise.
     */
    [[nodiscard]] constexpr auto hasValue() const noexcept -> bool {
        return reinterpret_cast<const T*>(&storage_) != nullptr;
    }

private:
    /**
     * @brief Retrieves a reference to the managed object.
     *
     * @return A reference to the managed object.
     */
    constexpr auto get() noexcept -> T& {
        return reinterpret_cast<T&>(storage_);
    }

    /**
     * @brief Retrieves a const reference to the managed object.
     *
     * @return A const reference to the managed object.
     */
    constexpr auto get() const noexcept -> const T& {
        return reinterpret_cast<const T&>(storage_);
    }

    /// Storage for the managed object, aligned to T's alignment requirements.
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
};

#endif  // ATOM_TYPE_NO_OFFSET_PTR_HPP