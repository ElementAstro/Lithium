#ifndef ATOM_TYPE_TRACKABLE_HPP
#define ATOM_TYPE_TRACKABLE_HPP

#include <exception>
#include <functional>
#include <mutex>
#include <utility>
#include <vector>

#include "atom/error/exception.hpp"
#include "atom/utils/cstring.hpp"

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
    explicit Trackable(T initialValue)
        : value_(std::move(initialValue)), notifyDeferred_(false) {}

    /**
     * @brief Subscribe a callback function to be called when the value changes.
     *
     * @param onChange The callback function to be called when the value
     * changes. It takes two const references: the old value and the new value.
     */
    void subscribe(std::function<void(const T&, const T&)> onChange) {
        std::scoped_lock lock(mutex_);
        observers_.emplace_back(std::move(onChange));
    }

    /**
     * @brief Unsubscribe all observer functions.
     */
    void unsubscribeAll() {
        std::scoped_lock lock(mutex_);
        observers_.clear();
    }

    /**
     * @brief Overloaded assignment operator to update the value and notify
     * observers.
     *
     * @param newValue The new value to be assigned.
     * @return Trackable& Reference to the trackable object.
     */
    Trackable& operator=(T newValue) {
        std::scoped_lock lock(mutex_);
        if (value_ != newValue) {
            T oldValue = std::exchange(value_, std::move(newValue));
            if (!notifyDeferred_) {
                notifyObservers(oldValue, value_);
            }
        }
        return *this;
    }

    /**
     * @brief Control whether notifications are deferred or not.
     *
     * @param defer If true, notifications will be deferred until
     * deferNotifications(false) is called.
     */
    void deferNotifications(bool defer) {
        std::scoped_lock lock(mutex_);
        notifyDeferred_ = defer;
        if (!defer) {
            notifyObservers(std::exchange(lastOldValue_, value_), value_);
        }
    }

    /**
     * @brief Overloaded += operator to increment the value and notify
     * observers.
     *
     * @param rhs The value to be added.
     * @return Trackable& Reference to the trackable object.
     */
    Trackable& operator+=(const T& rhs) {
        std::scoped_lock lock(mutex_);
        T newValue = value_ + rhs;
        if (value_ != newValue) {
            T oldValue = std::exchange(value_, std::move(newValue));
            if (!notifyDeferred_) {
                notifyObservers(oldValue, value_);
            }
        }
        return *this;
    }

    /**
     * @brief Conversion operator to convert the trackable object to its value
     * type.
     *
     * @return T The value of the trackable object.
     */
    operator T() const {
        std::scoped_lock lock(mutex_);
        return value_;
    }

private:
    T value_;  ///< The stored value.
    std::vector<std::function<void(const T&, const T&)>>
        observers_;             ///< List of observer functions.
    mutable std::mutex mutex_;  ///< Mutex for thread safety.
    bool notifyDeferred_;       ///< Flag to control deferred notifications.
    T lastOldValue_;            ///< Last old value for deferred notifications.

    /**
     * @brief Notifies all observers about the value change.
     *
     * @param oldVal The old value.
     * @param newVal The new value.
     */
    void notifyObservers(const T& oldVal, const T& newVal) {
        auto localObservers = observers_;
        mutex_.unlock();  // 解锁,以避免在调用回调时持有锁
        for (const auto& observer : localObservers) {
            try {
                observer(oldVal, newVal);
            } catch (const std::exception& e) {
                THROW_EXCEPTION(concat("Exception in observer.", e.what()));
            } catch (...) {
                THROW_EXCEPTION("Unknown exception in observer.");
            }
        }
        mutex_.lock();
    }
};

#endif