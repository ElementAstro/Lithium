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

#include <compare>      // For spaceship operator
#include <functional>   // For std::invoke
#include <optional>     // For std::optional
#include <stdexcept>    // For std::runtime_error
#include <type_traits>  // For std::is_nothrow_move_constructible_v
#include <utility>      // For std::forward

namespace atom::type {
/**
 * @brief A simple optional wrapper around std::optional.
 *
 * This class provides a wrapper around `std::optional` to represent an optional
 * value that may or may not be present. It supports basic operations such as
 * accessing the contained value, resetting, and applying transformations.
 *
 * @tparam T The type of the contained value.
 */
template <typename T>
class Optional {
private:
    std::optional<T> storage_;

public:
    /**
     * @brief Default constructor.
     *
     * Constructs an empty `Optional` object.
     */
    Optional() noexcept = default;

    /**
     * @brief Constructor with std::nullopt_t.
     *
     * Constructs an empty `Optional` object using std::nullopt.
     *
     * @param nullopt A nullopt_t instance.
     */
    Optional(std::nullopt_t) noexcept : storage_(std::nullopt) {}

    /**
     * @brief Constructor with a const reference.
     *
     * Constructs an `Optional` object containing the given value.
     *
     * @param value The value to be contained.
     */
    Optional(const T& value) : storage_(value) {}

    /**
     * @brief Constructor with an rvalue reference.
     *
     * Constructs an `Optional` object containing the given value, which is
     * moved into the object.
     *
     * @param value The value to be moved into the object.
     */
    Optional(T&& value) noexcept(std::is_nothrow_move_constructible_v<T>)
        : storage_(std::move(value)) {}

    /**
     * @brief Copy constructor.
     *
     * Constructs an `Optional` object as a copy of another `Optional` object.
     *
     * @param other The other `Optional` object to copy from.
     */
    Optional(const Optional& other) = default;

    /**
     * @brief Move constructor.
     *
     * Constructs an `Optional` object by moving from another `Optional` object.
     * The moved-from object will be empty after the move.
     *
     * @param other The other `Optional` object to move from.
     */
    Optional(Optional&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        : storage_(std::move(other.storage_)) {
        other.reset();  // Move the object to empty state
    }

    /**
     * @brief Destructor.
     *
     * Destroys the `Optional` object.
     */
    ~Optional() = default;

    /**
     * @brief Assignment operator with std::nullopt_t.
     *
     * Assigns an empty state to the `Optional` object.
     *
     * @param nullopt A nullopt_t instance.
     * @return A reference to this `Optional` object.
     */
    Optional& operator=(std::nullopt_t) noexcept {
        storage_ = std::nullopt;
        return *this;
    }

    /**
     * @brief Copy assignment operator.
     *
     * Assigns the value of another `Optional` object to this `Optional` object.
     *
     * @param other The other `Optional` object to copy from.
     * @return A reference to this `Optional` object.
     */
    Optional& operator=(const Optional& other) = default;

    /**
     * @brief Move assignment operator.
     *
     * Assigns the value of another `Optional` object to this `Optional` object
     * by moving. The moved-from object will be empty after the move.
     *
     * @param other The other `Optional` object to move from.
     * @return A reference to this `Optional` object.
     */
    Optional& operator=(Optional&& other) noexcept(
        std::is_nothrow_move_assignable_v<T>) {
        if (this != &other) {
            storage_ = std::move(other.storage_);
            other.reset();  // Move the object to empty state
        }
        return *this;
    }

    /**
     * @brief Assignment operator with a const reference.
     *
     * Assigns a new value to the `Optional` object.
     *
     * @param value The new value to assign.
     * @return A reference to this `Optional` object.
     */
    Optional& operator=(const T& value) {
        storage_ = value;
        return *this;
    }

    /**
     * @brief Assignment operator with an rvalue reference.
     *
     * Assigns a new value to the `Optional` object, which is moved into the
     * object.
     *
     * @param value The new value to move into the object.
     * @return A reference to this `Optional` object.
     */
    Optional& operator=(T&& value) noexcept(
        std::is_nothrow_move_assignable_v<T>) {
        storage_ = std::move(value);
        return *this;
    }

    /**
     * @brief Constructs a new value in the `Optional` object.
     *
     * Constructs a new value in-place within the `Optional` object using the
     * given arguments.
     *
     * @tparam Args The types of the arguments to forward to the constructor of
     * T.
     * @param args The arguments to forward.
     * @return A reference to the newly constructed value.
     */
    template <typename... Args>
    T& emplace(Args&&... args) {
        storage_.emplace(std::forward<Args>(args)...);
        return *storage_;
    }

    /**
     * @brief Checks if the `Optional` object contains a value.
     *
     * @return True if the `Optional` object contains a value, false otherwise.
     */
    constexpr explicit operator bool() const noexcept {
        return storage_.has_value();
    }

    /**
     * @brief Dereference operator for lvalue `Optional`.
     *
     * Accesses the contained value.
     *
     * @return A reference to the contained value.
     * @throw std::runtime_error if the `Optional` object is empty.
     */
    T& operator*() & {
        check_value();
        return *storage_;
    }

    /**
     * @brief Dereference operator for const lvalue `Optional`.
     *
     * Accesses the contained value.
     *
     * @return A const reference to the contained value.
     * @throw std::runtime_error if the `Optional` object is empty.
     */
    const T& operator*() const& {
        check_value();
        return *storage_;
    }

    /**
     * @brief Dereference operator for rvalue `Optional`.
     *
     * Accesses the contained value and moves it.
     *
     * @return An rvalue reference to the contained value.
     * @throw std::runtime_error if the `Optional` object is empty.
     */
    T&& operator*() && {
        check_value();
        return std::move(*storage_);
    }

    /**
     * @brief Member access operator for lvalue `Optional`.
     *
     * Accesses the contained value using the arrow operator.
     *
     * @return A pointer to the contained value.
     * @throw std::runtime_error if the `Optional` object is empty.
     */
    T* operator->() {
        check_value();
        return &(*storage_);
    }

    /**
     * @brief Member access operator for const lvalue `Optional`.
     *
     * Accesses the contained value using the arrow operator.
     *
     * @return A const pointer to the contained value.
     * @throw std::runtime_error if the `Optional` object is empty.
     */
    const T* operator->() const {
        check_value();
        return &(*storage_);
    }

    /**
     * @brief Accesses the contained value.
     *
     * @return A reference to the contained value.
     * @throw std::runtime_error if the `Optional` object is empty.
     */
    T& value() & {
        check_value();
        return *storage_;
    }

    /**
     * @brief Accesses the contained value for const lvalue `Optional`.
     *
     * @return A const reference to the contained value.
     * @throw std::runtime_error if the `Optional` object is empty.
     */
    const T& value() const& {
        check_value();
        return *storage_;
    }

    /**
     * @brief Accesses the contained value and moves it.
     *
     * @return An rvalue reference to the contained value.
     * @throw std::runtime_error if the `Optional` object is empty.
     */
    T&& value() && {
        check_value();
        return std::move(*storage_);
    }

    /**
     * @brief Returns the contained value or a default value.
     *
     * If the `Optional` object contains a value, it returns that value.
     * Otherwise, it returns the provided default value.
     *
     * @tparam U The type of the default value.
     * @param default_value The default value to return if the `Optional` is
     * empty.
     * @return The contained value if present, otherwise the default value.
     */
    template <typename U>
    T value_or(U&& default_value) const& {
        return storage_.value_or(std::forward<U>(default_value));
    }

    /**
     * @brief Returns the contained value or a default value (rvalue version).
     *
     * If the `Optional` object contains a value, it returns that value.
     * Otherwise, it returns the provided default value.
     *
     * @tparam U The type of the default value.
     * @param default_value The default value to return if the `Optional` is
     * empty.
     * @return The contained value if present, otherwise the default value.
     */
    template <typename U>
    T value_or(U&& default_value) && {
        return storage_.value_or(std::forward<U>(default_value));
    }

    /**
     * @brief Resets the `Optional` object to an empty state.
     *
     * This function clears the contained value, if any, leaving the `Optional`
     * object in an empty state.
     */
    void reset() noexcept { storage_.reset(); }

    /**
     * @brief Three-way comparison operator.
     *
     * Compares two `Optional` objects. This operator is defaulted, which means
     * it will use the comparison of the contained values.
     *
     * @param other The other `Optional` object to compare to.
     * @return A three-way comparison result.
     */
    auto operator<=>(const Optional&) const = default;

    /**
     * @brief Comparison with std::nullopt_t.
     *
     * Checks if the `Optional` object is equal to `std::nullopt`.
     *
     * @param nullopt A nullopt_t instance.
     * @return True if the `Optional` object is empty, false otherwise.
     */
    bool operator==(std::nullopt_t) const noexcept {
        return !storage_.has_value();
    }

    /**
     * @brief Comparison with std::nullopt_t (three-way comparison).
     *
     * Compares the `Optional` object with `std::nullopt`.
     *
     * @param nullopt A nullopt_t instance.
     * @return A three-way comparison result.
     */
    auto operator<=>(std::nullopt_t) const noexcept {
        return storage_.has_value() ? std::strong_ordering::greater
                                    : std::strong_ordering::equal;
    }

    /**
     * @brief Applies a function to the contained value, if present.
     *
     * If the `Optional` object contains a value, applies the function to that
     * value and returns a new `Optional` object with the result. Otherwise,
     * returns an empty `Optional`.
     *
     * @tparam F The type of the function.
     * @param f The function to apply.
     * @return An `Optional` object containing the result of applying the
     * function, or an empty `Optional`.
     */
    template <typename F>
    auto map(F&& f) const -> Optional<std::invoke_result_t<F, T>> {
        using ReturnType = std::invoke_result_t<F, T>;
        return storage_.has_value() ? Optional<ReturnType>(std::invoke(
                                          std::forward<F>(f), *storage_))
                                    : std::nullopt;
    }

    /**
     * @brief Applies a function to the contained value, if present.
     *
     * If the `Optional` object contains a value, applies the function to that
     * value and returns the result.
     *
     * @tparam F The type of the function.
     * @param f The function to apply.
     * @return The result of applying the function, or a default-constructed
     * value if the `Optional` is empty.
     */
    template <typename F>
    auto and_then(F&& f) const -> std::invoke_result_t<F, T> {
        if (storage_.has_value()) {
            return std::invoke(std::forward<F>(f), *storage_);
        }
        return std::invoke_result_t<F, T>();
    }

    /**
     * @brief Applies a function to the contained value and returns a new
     * `Optional` with the result.
     *
     * This function is an alias for `map`.
     *
     * @tparam F The type of the function.
     * @param f The function to apply.
     * @return An `Optional` object containing the result of applying the
     * function, or an empty `Optional`.
     */
    template <typename F>
    auto transform(F&& f) const -> Optional<std::invoke_result_t<F, T>> {
        return map(std::forward<F>(f));
    }

    /**
     * @brief Returns the contained value or invokes a function to generate a
     * default value.
     *
     * If the `Optional` object contains a value, returns that value.
     * Otherwise, invokes the provided function to generate a default value.
     *
     * @tparam F The type of the function.
     * @param f The function to invoke if the `Optional` is empty.
     * @return The contained value if present, otherwise the result of invoking
     * the function.
     */
    template <typename F>
    auto or_else(F&& f) const -> T {
        return storage_.has_value() ? *storage_
                                    : std::invoke(std::forward<F>(f));
    }

    /**
     * @brief Applies a function to the contained value and returns a new
     * `Optional` with the result or a default value.
     *
     * If the `Optional` object contains a value, applies the function to that
     * value and returns a new `Optional` with the result. Otherwise, returns a
     * new `Optional` containing the default value.
     *
     * @tparam F The type of the function.
     * @param f The function to apply if the `Optional` is not empty.
     * @param default_value The default value to use if the `Optional` is empty.
     * @return An `Optional` object containing the result of applying the
     * function, or the default value.
     */
    template <typename F>
    auto transform_or(F&& f, const T& default_value) const -> Optional<T> {
        if (storage_.has_value()) {
            return Optional<T>(std::invoke(std::forward<F>(f), *storage_));
        }
        return Optional<T>(default_value);
    }

    /**
     * @brief Applies a function to the contained value and returns the result.
     *
     * If the `Optional` object contains a value, applies the function to that
     * value and returns the result. Otherwise, returns a default-constructed
     * value.
     *
     * @tparam F The type of the function.
     * @param f The function to apply.
     * @return The result of applying the function, or a default-constructed
     * value if the `Optional` is empty.
     */
    template <typename F>
    auto flat_map(F&& f) const -> std::invoke_result_t<F, T> {
        if (storage_.has_value()) {
            return std::invoke(std::forward<F>(f), *storage_);
        }
        return std::invoke_result_t<F, T>();
    }

private:
    /**
     * @brief Checks if the `Optional` object contains a value.
     *
     * Throws an exception if the `Optional` is empty.
     *
     * @throw std::runtime_error if the `Optional` object is empty.
     */
    void check_value() const {
        if (!storage_.has_value()) {
            throw std::runtime_error("Optional has no value");
        }
    }
};
}  // namespace atom::type

#endif  // ATOM_TYPE_OPTIONAL_HPP
