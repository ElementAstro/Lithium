/*
 * trackable.hpp
 *
 * Copyright (C) 2023-2024 Max Qian <lightapt.com>
 */

/*************************************************

Date: 2024-2-10

Description: Trackable Object

**************************************************/

#ifndef ATOM_TYPE_TRACKABLE_HPP
#define ATOM_TYPE_TRACKABLE_HPP

#include <functional>
#include <mutex>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/function/abi.hpp"

/**
 * @brief A class template for creating trackable objects that notify observers
 * when their value changes.
 *
 * @tparam T The type of the value being tracked.
 */

template <typename T>
class Trackable {
public:
    /**
     * @brief Constructor to initialize the trackable object with an initial
     * value.
     *
     * @param initialValue The initial value of the trackable object.
     */
    explicit Trackable(T initialValue) : value_(std::move(initialValue)) {}

    /**
     * @brief Subscribe a callback function to be called when the value changes.
     *
     * @param onChange The callback function to be called when the value
     * changes. It takes two const references: the old value and the new value.
     */
    void subscribe(std::function<void(const T&, const T&)> onChange) {
        std::lock_guard lock(mutex_);
        observers_.emplace_back(std::move(onChange));
    }

    void setOnChangeCallback(std::function<void(const T&)> onChange) {
        std::lock_guard lock(mutex_);
        onChangeCallback_ = std::move(onChange);
    }

    /**
     * @brief Unsubscribe all observer functions.
     */
    void unsubscribeAll() {
        std::lock_guard lock(mutex_);
        observers_.clear();
    }

    /**
     * @brief Checks if there are any subscribers.
     *
     * @return true if there are subscribers, false otherwise.
     */
    auto hasSubscribers() const -> bool {
        std::lock_guard lock(mutex_);
        return !observers_.empty();
    }

    /**
     * @brief Get the current value of the trackable object.
     *
     * @return const T& A const reference to the current value.
     */
    auto get() const -> const T& {
        std::lock_guard lock(mutex_);
        return value_;
    }

    /**
     * @brief Get the demangled type name of the stored value.
     *
     * @return std::string The demangled type name of the value.
     */
    auto getTypeName() const -> std::string {
        return atom::meta::DemangleHelper::demangleType<T>();
    }

    /**
     * @brief Overloaded assignment operator to update the value and notify
     * observers.
     *
     * @param newValue The new value to be assigned.
     * @return Trackable& Reference to the trackable object.
     */
    auto operator=(T newValue) -> Trackable& {
        std::lock_guard lock(mutex_);
        if (value_ != newValue) {
            T oldValue = std::exchange(value_, std::move(newValue));
            if (!notifyDeferred_) {
                notifyObservers(oldValue, value_);
            } else {
                lastOldValue_ = std::move(oldValue);
            }
        }
        return *this;
    }

    /**
     * @brief Overloaded += operator to increment the value and notify
     * observers.
     *
     * @param rhs The value to be added.
     * @return Trackable& Reference to the trackable object.
     */
    auto operator+=(const T& rhs) -> Trackable& {
        if constexpr (std::is_arithmetic_v<T>) {
            return applyOperation(rhs, std::plus<>{});
        } else {
            return appendVector(rhs);
        }
    }

    /**
     * @brief Overloaded -= operator to decrement the value and notify
     * observers.
     *
     * @param rhs The value to be subtracted.
     * @return Trackable& Reference to the trackable object.
     */
    auto operator-=(const T& rhs) -> Trackable& {
        return applyOperation(rhs, std::minus<>{});
    }

    /**
     * @brief Overloaded *= operator to multiply the value and notify
     * observers.
     *
     * @param rhs The value to be multiplied by.
     * @return Trackable& Reference to the trackable object.
     */
    auto operator*=(const T& rhs) -> Trackable& {
        return applyOperation(rhs, std::multiplies<>{});
    }

    /**
     * @brief Overloaded /= operator to divide the value and notify
     * observers.
     *
     * @param rhs The value to be divided by.
     * @return Trackable& Reference to the trackable object.
     */
    auto operator/=(const T& rhs) -> Trackable& {
        return applyOperation(rhs, std::divides<>{});
    }

    /**
     * @brief Conversion operator to convert the trackable object to its value
     * type.
     *
     * @return T The value of the trackable object.
     */
    explicit operator T() const { return get(); }

    /**
     * @brief Control whether notifications are deferred or not.
     *
     * @param defer If true, notifications will be deferred until
     * deferNotifications(false) is called.
     */
    void deferNotifications(bool defer) {
        std::lock_guard lock(mutex_);
        notifyDeferred_ = defer;
        if (!defer && lastOldValue_.has_value()) {
            notifyObservers(*lastOldValue_, value_);
            lastOldValue_.reset();
        }
    }

    /**
     * @brief A scope-based notification deferrer.
     *
     * Usage:
     * {
     *     auto deferrer = myTrackable.deferScoped();
     *     // multiple operations here...
     * } // Notifications sent here, if any changes occurred.
     */
    [[nodiscard]] auto deferScoped() {
        deferNotifications(true);
        return std::shared_ptr<void>(
            nullptr, [this](...) { this->deferNotifications(false); });
    }

private:
    T value_;  ///< The stored value.
    std::vector<std::function<void(const T&, const T&)>>
        observers_;             ///< List of observer functions.
    mutable std::mutex mutex_;  ///< Mutex for thread safety.
    bool notifyDeferred_{};     ///< Flag to control deferred notifications.
    std::optional<T>
        lastOldValue_;  ///< Last old value for deferred notifications.
    std::function<void(const T&)>
        onChangeCallback_;  ///< Callback for value changes.

    /**
     * @brief Notifies all observers about the value change.
     *
     * @param oldVal The old value.
     * @param newVal The new value.
     */
    void notifyObservers(const T& oldVal, const T& newVal) {
        // Make a local copy of the observers to avoid holding the lock while
        // notifying.
        auto localObservers = observers_;
        auto localOnChangeCallback = onChangeCallback_;
        mutex_.unlock();  // Unlock before notifying to prevent deadlocks.
        for (const auto& observer : localObservers) {
            try {
                observer(oldVal, newVal);
            } catch (const std::exception& e) {
                mutex_.lock();
                THROW_EXCEPTION("Exception in observer: ", e.what());
            } catch (...) {
                mutex_.lock();
                THROW_EXCEPTION("Unknown exception in observer.");
            }
        }
        if (localOnChangeCallback) {
            try {
                localOnChangeCallback(newVal);
            } catch (const std::exception& e) {
                mutex_.lock();
                THROW_EXCEPTION("Exception in onChangeCallback: ", e.what());
            } catch (...) {
                mutex_.lock();
                THROW_EXCEPTION("Unknown exception in onChangeCallback.");
            }
        }
        mutex_.lock();
    }

    /**
     * @brief Applies an arithmetic operation and notifies observers.
     *
     * @param rhs The right-hand side operand.
     * @param op The operation to apply.
     * @return Trackable& Reference to the trackable object.
     */
    template <typename Operation>
    auto applyOperation(const T& rhs, Operation op) -> Trackable& {
        std::lock_guard lock(mutex_);
        T newValue = op(value_, rhs);
        if (value_ != newValue) {
            T oldValue = std::exchange(value_, std::move(newValue));
            if (!notifyDeferred_) {
                notifyObservers(oldValue, value_);
            } else {
                lastOldValue_ = std::move(oldValue);
            }
        }
        return *this;
    }

    // Special handling for vectors to append elements
    template <typename U = T>
    auto appendVector(const U& rhs) ->
        typename std::enable_if_t<
            std::is_same_v<U, std::vector<typename U::value_type,
                                          typename U::allocator_type>>,
            Trackable&> {
        std::lock_guard lock(mutex_);
        if (!rhs.empty()) {
            T oldValue = value_;
            value_.insert(value_.end(), rhs.begin(), rhs.end());
            if (!notifyDeferred_) {
                notifyObservers(oldValue, value_);
            } else {
                lastOldValue_ = std::move(oldValue);
            }
        }
        return *this;
    }
};

#endif
