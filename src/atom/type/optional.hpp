/*
 * optional.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: A simple implementation of optional. Using modern C++ features.

**************************************************/

#ifndef ATOM_TYPE_OPTIONAL_HPP
#define ATOM_TYPE_OPTIONAL_HPP

#include <stdexcept>
#include <type_traits>
#include <utility>

/**
 * @brief A simple implementation of an optional type.
 *
 * This class template provides an optional container that may or may not
 * contain a value.
 *
 * @tparam T The type of the object to manage.
 */
template <typename T>
class Optional {
private:
    /// Storage for the managed object, aligned to T's alignment requirements.
    using StorageType =
        typename std::aligned_storage<sizeof(T), alignof(T)>::type;
    StorageType storage_;
    bool hasValue_;

public:
    /**
     * @brief Default constructor. Constructs an empty Optional.
     */
    Optional() noexcept : hasValue_(false) {}

    /**
     * @brief Constructs an Optional with a value.
     *
     * @param value The value to store in the Optional.
     */
    explicit Optional(const T &value) {
        new (&storage_) T(value);
        hasValue_ = true;
    }

    /**
     * @brief Constructs an Optional with a moved value.
     *
     * @param value The value to move into the Optional.
     *
     * @note This constructor is noexcept if T's move constructor is noexcept.
     */
    explicit Optional(T &&value) noexcept(
        std::is_nothrow_move_constructible<T>::value) {
        new (&storage_) T(std::move(value));
        hasValue_ = true;
    }

    /**
     * @brief Copy constructor.
     *
     * @param other The other Optional to copy from.
     */
    Optional(const Optional &other) {
        if (other.hasValue_) {
            new (&storage_) T(other.value());
            hasValue_ = true;
        } else {
            hasValue_ = false;
        }
    }

    /**
     * @brief Move constructor.
     *
     * @param other The other Optional to move from.
     *
     * @note This constructor is noexcept if T's move constructor is noexcept.
     */
    Optional(Optional &&other) noexcept(
        std::is_nothrow_move_constructible<T>::value) {
        if (other.hasValue_) {
            new (&storage_) T(std::move(other.value()));
            hasValue_ = true;
        } else {
            hasValue_ = false;
        }
    }

    /**
     * @brief Destructor. Destroys the contained value if it exists.
     */
    ~Optional() { reset(); }

    /**
     * @brief Copy assignment operator.
     *
     * @param other The other Optional to copy from.
     * @return A reference to this Optional.
     */
    Optional &operator=(const Optional &other) {
        if (this != &other) {
            reset();
            if (other.hasValue_) {
                new (&storage_) T(other.value());
                hasValue_ = true;
            } else {
                hasValue_ = false;
            }
        }
        return *this;
    }

    /**
     * @brief Move assignment operator.
     *
     * @param other The other Optional to move from.
     * @return A reference to this Optional.
     *
     * @note This operator is noexcept if T's move assignment operator is
     * noexcept.
     */
    Optional &operator=(Optional &&other) noexcept(
        std::is_nothrow_move_assignable<T>::value) {
        if (this != &other) {
            reset();
            if (other.hasValue_) {
                new (&storage_) T(std::move(other.value()));
                hasValue_ = true;
            } else {
                hasValue_ = false;
            }
        }
        return *this;
    }

    /**
     * @brief Constructs the contained value in-place.
     *
     * @tparam Args The types of the arguments to pass to T's constructor.
     * @param args The arguments to pass to T's constructor.
     */
    template <typename... Args>
    void emplace(Args &&...args) {
        reset();
        new (&storage_) T(std::forward<Args>(args)...);
        hasValue_ = true;
    }

    /**
     * @brief Provides pointer-like access to the contained value.
     *
     * @return A pointer to the contained value.
     *
     * @throws std::runtime_error if the Optional has no value.
     */
    T *operator->() {
        if (!hasValue_) {
            throw std::runtime_error("Optional has no value");
        }
        return reinterpret_cast<T *>(&storage_);
    }

    /**
     * @brief Provides const pointer-like access to the contained value.
     *
     * @return A const pointer to the contained value.
     *
     * @throws std::runtime_error if the Optional has no value.
     */
    const T *operator->() const {
        if (!hasValue_) {
            throw std::runtime_error("Optional has no value");
        }
        return reinterpret_cast<const T *>(&storage_);
    }

    /**
     * @brief Dereferences the contained value.
     *
     * @return A reference to the contained value.
     *
     * @throws std::runtime_error if the Optional has no value.
     */
    T &operator*() {
        if (!hasValue_) {
            throw std::runtime_error("Optional has no value");
        }
        return *reinterpret_cast<T *>(&storage_);
    }

    /**
     * @brief Dereferences the contained value.
     *
     * @return A const reference to the contained value.
     *
     * @throws std::runtime_error if the Optional has no value.
     */
    const T &operator*() const {
        if (!hasValue_) {
            throw std::runtime_error("Optional has no value");
        }
        return *reinterpret_cast<const T *>(&storage_);
    }

    /**
     * @brief Destroys the contained value if it exists.
     */
    void reset() noexcept {
        if (hasValue_) {
            reinterpret_cast<T *>(&storage_)->~T();
            hasValue_ = false;
        }
    }

    /**
     * @brief Returns a reference to the contained value.
     *
     * @return A reference to the contained value.
     *
     * @throws std::runtime_error if the Optional has no value.
     */
    T &value() {
        if (!hasValue_) {
            throw std::runtime_error("Optional has no value");
        }
        return *reinterpret_cast<T *>(&storage_);
    }

    /**
     * @brief Returns a const reference to the contained value.
     *
     * @return A const reference to the contained value.
     *
     * @throws std::runtime_error if the Optional has no value.
     */
    const T &value() const {
        if (!hasValue_) {
            throw std::runtime_error("Optional has no value");
        }
        return *reinterpret_cast<const T *>(&storage_);
    }

    /**
     * @brief Checks whether the Optional contains a value.
     *
     * @return true if the Optional contains a value, false otherwise.
     */
    explicit operator bool() const noexcept { return hasValue_; }

    /**
     * @brief Equality operator.
     *
     * @param other The other Optional to compare with.
     * @return true if both Optionals are equal, false otherwise.
     */
    bool operator==(const Optional &other) const {
        if (hasValue_ != other.hasValue_) {
            return false;
        }
        if (hasValue_) {
            return value() == other.value();
        }
        return true;
    }

    /**
     * @brief Inequality operator.
     *
     * @param other The other Optional to compare with.
     * @return true if both Optionals are not equal, false otherwise.
     */
    bool operator!=(const Optional &other) const { return !(*this == other); }

    /**
     * @brief Returns the contained value if it exists, otherwise returns the
     * provided default value.
     *
     * @param defaultValue The value to return if the Optional has no value.
     * @return The contained value or the default value.
     */
    auto valueOr(const T &defaultValue) const -> T {
        return hasValue_ ? value() : defaultValue;
    }

#if __cplusplus >= 201703L
    /**
     * @brief Returns the contained value if it exists, otherwise returns the
     * provided default value.
     *
     * @tparam U The type of the default value.
     * @param defaultValue The value to return if the Optional has no value.
     * @return The contained value or the default value.
     */
    template <typename U>
    auto valueOr(U &&defaultValue) const -> T {
        return hasValue_ ? value()
                         : static_cast<T>(std::forward<U>(defaultValue));
    }

    /**
     * @brief Transforms the contained value using the provided function if it
     * exists.
     *
     * @tparam F The type of the function to apply.
     * @param f The function to apply to the contained value.
     * @return An Optional containing the result of the function or an empty
     * Optional.
     */
    template <typename F>
    auto map(F &&f) const -> Optional<decltype(f(value()))> {
        using ReturnType = decltype(f(value()));
        if (hasValue_) {
            return Optional<ReturnType>(f(value()));
        }
        return Optional<ReturnType>();
    }
#endif

#if __cplusplus >= 202002L
    /**
     * @brief Transforms the contained value using the provided function if it
     * exists.
     *
     * @tparam F The type of the function to apply.
     * @param f The function to apply to the contained value.
     * @return The result of the function or an empty Optional.
     */
    template <typename F>
    auto andThen(F &&f) const -> decltype(f(value())) {
        using ReturnType = decltype(f(value()));
        if (hasValue_) {
            return f(value());
        }
        return ReturnType();
    }
#endif
};

#endif  // ATOM_TYPE_OPTIONAL_HPP
