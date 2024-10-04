#ifndef ATOM_ASYNC_PROMISE_HPP
#define ATOM_ASYNC_PROMISE_HPP

#include <atomic>
#include <exception>
#include <functional>
#include <future>
#include <vector>

#include "atom/async/future.hpp"

namespace atom::async {

/**
 * @class PromiseCancelledException
 * @brief Exception thrown when a promise is cancelled.
 */
class PromiseCancelledException : public atom::error::RuntimeError {
public:
    using atom::error::RuntimeError::RuntimeError;
};

/**
 * @def THROW_PROMISE_CANCELLED_EXCEPTION
 * @brief Macro to throw a PromiseCancelledException with file, line, and
 * function information.
 */
#define THROW_PROMISE_CANCELLED_EXCEPTION(...)                      \
    throw PromiseCancelledException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                    ATOM_FUNC_NAME, __VA_ARGS__);

/**
 * @def THROW_NESTED_PROMISE_CANCELLED_EXCEPTION
 * @brief Macro to rethrow a nested PromiseCancelledException with file, line,
 * and function information.
 */
#define THROW_NESTED_PROMISE_CANCELLED_EXCEPTION(...)   \
    PromiseCancelledException::rethrowNested(           \
        ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, \
        "Promise cancelled: " __VA_ARGS__);

/**
 * @class EnhancedPromise
 * @brief A template class that extends the standard promise with additional
 * features.
 * @tparam T The type of the value that the promise will hold.
 */
template <typename T>
class EnhancedPromise {
public:
    /**
     * @brief Constructor that initializes the promise and shared future.
     */
    EnhancedPromise();

    /**
     * @brief Gets the enhanced future associated with this promise.
     * @return An EnhancedFuture object.
     */
    auto getEnhancedFuture() -> EnhancedFuture<T>;

    /**
     * @brief Sets the value of the promise.
     * @param value The value to set.
     * @throws PromiseCancelledException if the promise has been cancelled.
     */
    void setValue(T value);

    /**
     * @brief Sets an exception for the promise.
     * @param exception The exception to set.
     * @throws PromiseCancelledException if the promise has been cancelled.
     */
    void setException(std::exception_ptr exception);

    /**
     * @brief Adds a callback to be called when the promise is completed.
     * @tparam F The type of the callback function.
     * @param func The callback function to add.
     */
    template <typename F>
    void onComplete(F &&func);

    /**
     * @brief Cancels the promise.
     */
    void cancel();

    /**
     * @brief Checks if the promise has been cancelled.
     * @return True if the promise has been cancelled, false otherwise.
     */
    [[nodiscard]] auto isCancelled() const -> bool;

    /**
     * @brief Gets the shared future associated with this promise.
     * @return A shared future object.
     */
    auto getFuture() -> std::shared_future<T>;

private:
    /**
     * @brief Runs all the registered callbacks.
     */
    void runCallbacks();

    std::promise<T> promise_;  ///< The underlying promise object.
    std::shared_future<T>
        future_;  ///< The shared future associated with the promise.
    std::vector<std::function<void(T)>>
        callbacks_;  ///< List of callbacks to be called on completion.
    std::atomic<bool>
        cancelled_;  ///< Flag indicating if the promise has been cancelled.
};

/**
 * @class EnhancedPromise<void>
 * @brief Specialization of the EnhancedPromise class for void type.
 */
template <>
class EnhancedPromise<void> {
public:
    /**
     * @brief Constructor that initializes the promise and shared future.
     */
    EnhancedPromise();

    /**
     * @brief Gets the enhanced future associated with this promise.
     * @return An EnhancedFuture object.
     */
    auto getEnhancedFuture() -> EnhancedFuture<void>;

    /**
     * @brief Sets the value of the promise.
     * @throws PromiseCancelledException if the promise has been cancelled.
     */
    void setValue();

    /**
     * @brief Sets an exception for the promise.
     * @param exception The exception to set.
     * @throws PromiseCancelledException if the promise has been cancelled.
     */
    void setException(std::exception_ptr exception);

    /**
     * @brief Adds a callback to be called when the promise is completed.
     * @tparam F The type of the callback function.
     * @param func The callback function to add.
     */
    template <typename F>
    void onComplete(F &&func);

    /**
     * @brief Cancels the promise.
     */
    void cancel();

    /**
     * @brief Checks if the promise has been cancelled.
     * @return True if the promise has been cancelled, false otherwise.
     */
    [[nodiscard]] auto isCancelled() const -> bool;

    /**
     * @brief Gets the shared future associated with this promise.
     * @return A shared future object.
     */
    auto getFuture() -> std::shared_future<void> { return future_; }

private:
    /**
     * @brief Runs all the registered callbacks.
     */
    void runCallbacks();

    std::promise<void> promise_;  ///< The underlying promise object.
    std::shared_future<void>
        future_;  ///< The shared future associated with the promise.
    std::vector<std::function<void()>>
        callbacks_;  ///< List of callbacks to be called on completion.
    std::atomic<bool>
        cancelled_;  ///< Flag indicating if the promise has been cancelled.
};

template <typename T>
EnhancedPromise<T>::EnhancedPromise()
    : future_(promise_.get_future().share()), cancelled_(false) {}

template <typename T>
auto EnhancedPromise<T>::getEnhancedFuture() -> EnhancedFuture<T> {
    return EnhancedFuture<T>(future_);
}

template <typename T>
void EnhancedPromise<T>::setValue(T value) {
    if (isCancelled()) {
        THROW_PROMISE_CANCELLED_EXCEPTION(
            "Cannot set value, promise was cancelled.");
    }
    promise_.set_value(value);
    runCallbacks();  // Execute callbacks
}

template <typename T>
void EnhancedPromise<T>::setException(std::exception_ptr exception) {
    if (isCancelled()) {
        THROW_PROMISE_CANCELLED_EXCEPTION(
            "Cannot set exception, promise was cancelled.");
    }
    promise_.set_exception(exception);
    runCallbacks();  // Execute callbacks
}

template <typename T>
template <typename F>
void EnhancedPromise<T>::onComplete(F &&func) {
    if (isCancelled()) {
        return;  // No callbacks should be added if the promise is cancelled
    }
    callbacks_.emplace_back(std::forward<F>(func));

    // If the promise is already set, run the callback immediately
    if (future_.valid() && future_.wait_for(std::chrono::seconds(0)) ==
                               std::future_status::ready) {
        runCallbacks();
    }
}

template <typename T>
void EnhancedPromise<T>::cancel() {
    cancelled_ = true;
}

template <typename T>
auto EnhancedPromise<T>::isCancelled() const -> bool {
    return cancelled_.load();
}

template <typename T>
auto EnhancedPromise<T>::getFuture() -> std::shared_future<T> {
    return future_;
}

template <typename T>
void EnhancedPromise<T>::runCallbacks() {
    if (isCancelled()) {
        return;
    }
    if (future_.valid() && future_.wait_for(std::chrono::seconds(0)) ==
                               std::future_status::ready) {
        try {
            T value =
                future_.get();  // Get the value and pass it to the callbacks
            for (auto &callback : callbacks_) {
                callback(value);
            }
        } catch (...) {
            // Handle the case where the future contains an exception.
            // We don't invoke callbacks in this case.
        }
    }
}

EnhancedPromise<void>::EnhancedPromise()
    : future_(promise_.get_future().share()), cancelled_(false) {}

auto EnhancedPromise<void>::getEnhancedFuture() -> EnhancedFuture<void> {
    return EnhancedFuture<void>(future_);
}

void EnhancedPromise<void>::setValue() {
    if (isCancelled()) {
        THROW_PROMISE_CANCELLED_EXCEPTION(
            "Cannot set value, promise was cancelled.");
    }
    promise_.set_value();
    runCallbacks();  // Execute callbacks
}

void EnhancedPromise<void>::setException(std::exception_ptr exception) {
    if (isCancelled()) {
        THROW_PROMISE_CANCELLED_EXCEPTION(
            "Cannot set exception, promise was cancelled.");
    }
    promise_.set_exception(exception);
    runCallbacks();  // Execute callbacks
}

template <typename F>
void EnhancedPromise<void>::onComplete(F &&func) {
    if (isCancelled()) {
        return;  // No callbacks should be added if the promise is cancelled
    }
    callbacks_.emplace_back(std::forward<F>(func));

    // If the promise is already set, run the callback immediately
    if (future_.valid() && future_.wait_for(std::chrono::seconds(0)) ==
                               std::future_status::ready) {
        runCallbacks();
    }
}

void EnhancedPromise<void>::cancel() { cancelled_ = true; }

auto EnhancedPromise<void>::isCancelled() const -> bool {
    return cancelled_.load();
}

void EnhancedPromise<void>::runCallbacks() {
    if (isCancelled()) {
        return;
    }
    if (future_.valid() && future_.wait_for(std::chrono::seconds(0)) ==
                               std::future_status::ready) {
        try {
            future_.get();  // Get the value and execute callbacks (for void,
                            // there's no value to pass)
            for (auto &callback : callbacks_) {
                callback();
            }
        } catch (...) {
            // Handle the case where the future contains an exception.
            // We don't invoke callbacks in this case.
        }
    }
}

}  // namespace atom::async

#endif  // ATOM_ASYNC_PROMISE_HPP
