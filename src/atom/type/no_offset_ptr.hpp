/*
 * no_offset_ptr.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-4-3

Description: No Offset Pointer

**************************************************/

#ifndef ATOM_TYPE_NoOffsetPtr_HPP
#define ATOM_TYPE_NoOffsetPtr_HPP

#include <cassert>
#include <type_traits>
#include <utility>

/**
 * @brief A lightweight pointer-like class that manages an object of type T
 * without dynamic memory allocation.
 *
 * @tparam T The type of the object to manage.
 */
template <typename T>
class UnshiftedPtr {
public:
    /**
     * @brief Default constructor. Constructs the managed object using T's
     * default constructor.
     *
     * @note This constructor is noexcept if T's default constructor is
     * noexcept.
     */
    UnshiftedPtr() noexcept(std::is_nothrow_default_constructible_v<T>) {
        new (&storage_) T;
    }

    /**
     * @brief Copy constructor. Constructs the managed object by copying the
     * given value.
     *
     * @param value The value to copy.
     *
     * @note This constructor is noexcept if T's copy constructor is noexcept.
     */
    explicit UnshiftedPtr(const T& value) noexcept(
        std::is_nothrow_copy_constructible_v<T>) {
        new (&storage_) T(value);
    }

    /**
     * @brief Copy constructor. Constructs the managed object by copying from
     * another UnshiftedPtr.
     *
     * @param other The other UnshiftedPtr to copy from.
     *
     * @note This constructor is noexcept if T's copy constructor is noexcept.
     */
    UnshiftedPtr(const UnshiftedPtr& other) noexcept(
        std::is_nothrow_copy_constructible_v<T>) {
        new (&storage_) T(other.get());
    }

    /**
     * @brief Move constructor. Constructs the managed object by moving from
     * another UnshiftedPtr.
     *
     * @param other The other UnshiftedPtr to move from.
     *
     * @note This constructor is noexcept if T's move constructor is noexcept.
     */
    UnshiftedPtr(UnshiftedPtr&& other) noexcept(
        std::is_nothrow_move_constructible_v<T>) {
        new (&storage_) T(std::move(other.get()));
    }

    /**
     * @brief Destructor. Destroys the managed object.
     */
    ~UnshiftedPtr() { get().~T(); }

    /**
     * @brief Copy assignment operator. Copies the value from another
     * UnshiftedPtr.
     *
     * @param other The other UnshiftedPtr to copy from.
     * @return A reference to this UnshiftedPtr.
     *
     * @note This operator is noexcept if T's copy assignment operator is
     * noexcept.
     */
    auto operator=(const UnshiftedPtr& other) noexcept(
        std::is_nothrow_copy_assignable_v<T>) -> UnshiftedPtr& {
        if (this != &other) {
            get() = other.get();
        }
        return *this;
    }

    /**
     * @brief Move assignment operator. Moves the value from another
     * UnshiftedPtr.
     *
     * @param other The other UnshiftedPtr to move from.
     * @return A reference to this UnshiftedPtr.
     *
     * @note This operator is noexcept if T's move assignment operator is
     * noexcept.
     */
    auto operator=(UnshiftedPtr&& other) noexcept(
        std::is_nothrow_move_assignable_v<T>) -> UnshiftedPtr& {
        if (this != &other) {
            get() = std::move(other.get());
        }
        return *this;
    }

    /**
     * @brief Provides pointer-like access to the managed object.
     *
     * @return A pointer to the managed object.
     */
    auto operator->() noexcept -> T* { return &get(); }

    /**
     * @brief Provides const pointer-like access to the managed object.
     *
     * @return A const pointer to the managed object.
     */
    auto operator->() const noexcept -> const T* { return &get(); }

    /**
     * @brief Dereferences the managed object.
     *
     * @return A reference to the managed object.
     */
    auto operator*() noexcept -> T& { return get(); }

    /**
     * @brief Dereferences the managed object.
     *
     * @return A const reference to the managed object.
     */
    auto operator*() const noexcept -> const T& { return get(); }

private:
    /**
     * @brief Retrieves a reference to the managed object.
     *
     * @return A reference to the managed object.
     */
    auto get() noexcept -> T& { return reinterpret_cast<T&>(storage_); }

    /**
     * @brief Retrieves a const reference to the managed object.
     *
     * @return A const reference to the managed object.
     */
    auto get() const noexcept -> const T& {
        return reinterpret_cast<const T&>(storage_);
    }

    /// Storage for the managed object, aligned to T's alignment requirements.
    std::aligned_storage_t<sizeof(T), alignof(T)> storage_;
};

#endif
