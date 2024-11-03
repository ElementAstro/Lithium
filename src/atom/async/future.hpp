#ifndef ATOM_ASYNC_FUTURE_HPP
#define ATOM_ASYNC_FUTURE_HPP

#include <functional>
#include <future>
#include <optional>

#include "atom/error/exception.hpp"

namespace atom::async {

/**
 * @class InvalidFutureException
 * @brief Exception thrown when an invalid future is encountered.
 */
class InvalidFutureException : public atom::error::RuntimeError {
public:
    using atom::error::RuntimeError::RuntimeError;
};

/**
 * @def THROW_INVALID_FUTURE_EXCEPTION
 * @brief Macro to throw an InvalidFutureException with file, line, and function
 * information.
 */
#define THROW_INVALID_FUTURE_EXCEPTION(...)                      \
    throw InvalidFutureException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                 ATOM_FUNC_NAME, __VA_ARGS__);

/**
 * @def THROW_NESTED_INVALID_FUTURE_EXCEPTION
 * @brief Macro to rethrow a nested InvalidFutureException with file, line, and
 * function information.
 */
#define THROW_NESTED_INVALID_FUTURE_EXCEPTION(...)                        \
    InvalidFutureException::rethrowNested(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                          ATOM_FUNC_NAME,                 \
                                          "Invalid future: " __VA_ARGS__);

/**
 * @class EnhancedFuture
 * @brief A template class that extends the standard future with additional
 * features.
 * @tparam T The type of the value that the future will hold.
 */
template <typename T>
class EnhancedFuture {
public:
    /**
     * @brief Constructs an EnhancedFuture from a shared future.
     * @param fut The shared future to wrap.
     */
    explicit EnhancedFuture(std::shared_future<T> &&fut)
        : future_(std::move(fut)), cancelled_(false) {}

    explicit EnhancedFuture(const std::shared_future<T> &fut)
        : future_(fut), cancelled_(false) {}

    /**
     * @brief Chains another operation to be called after the future is done.
     * @tparam F The type of the function to call.
     * @param func The function to call when the future is done.
     * @return An EnhancedFuture for the result of the function.
     */
    template <typename F>
    auto then(F &&func) {
        using ResultType = std::invoke_result_t<F, T>;
        return EnhancedFuture<ResultType>(
            std::async(std::launch::async, [fut = future_,
                                            func = std::forward<F>(
                                                func)]() mutable {
                if (fut.valid()) {
                    return func(fut.get());
                }
                THROW_INVALID_FUTURE_EXCEPTION(
                    "Future is invalid or cancelled");
            }).share());
    }

    /**
     * @brief Waits for the future with a timeout and auto-cancels if not ready.
     * @param timeout The timeout duration.
     * @return An optional containing the value if ready, or nullopt if timed
     * out.
     */
    auto waitFor(std::chrono::milliseconds timeout) -> std::optional<T> {
        if (future_.wait_for(timeout) == std::future_status::ready &&
            !cancelled_) {
            return future_.get();
        }
        cancel();
        return std::nullopt;
    }

    /**
     * @brief Checks if the future is done.
     * @return True if the future is done, false otherwise.
     */
    [[nodiscard]] auto isDone() const -> bool {
        return future_.wait_for(std::chrono::milliseconds(0)) ==
               std::future_status::ready;
    }

    /**
     * @brief Sets a completion callback to be called when the future is done.
     * @tparam F The type of the callback function.
     * @param func The callback function to add.
     */
    template <typename F>
    void onComplete(F &&func) {
        if (!cancelled_) {
            callbacks_.emplace_back(std::forward<F>(func));
            std::async(std::launch::async, [this]() {
                try {
                    if (future_.valid()) {
                        auto result = future_.get();
                        for (auto &callback : callbacks_) {
                            callback(result);
                        }
                    }
                } catch (const std::exception &e) {
                }
            }).get();
        }
    }

    /**
     * @brief Waits synchronously for the future to complete.
     * @return The value of the future.
     * @throws InvalidFutureException if the future is cancelled.
     */
    auto wait() -> T {
        if (cancelled_) {
            THROW_OBJ_NOT_EXIST("Future has been cancelled");
        }
        return future_.get();
    }

    template <typename F>
    auto catching(F &&func) {
        using ResultType = T;
        auto sharedFuture = std::make_shared<std::shared_future<T>>(future_);
        return EnhancedFuture<ResultType>(
            std::async(std::launch::async, [sharedFuture,
                                            func = std::forward<F>(
                                                func)]() mutable {
                try {
                    if (sharedFuture->valid()) {
                        return sharedFuture->get();
                    }
                    THROW_INVALID_FUTURE_EXCEPTION(
                        "Future is invalid or cancelled");
                } catch (...) {
                    return func(std::current_exception());
                }
            }).share());
    }

    /**
     * @brief Cancels the future.
     */
    void cancel() { cancelled_ = true; }

    /**
     * @brief Checks if the future has been cancelled.
     * @return True if the future has been cancelled, false otherwise.
     */
    [[nodiscard]] auto isCancelled() const -> bool { return cancelled_; }

    /**
     * @brief Gets the exception associated with the future, if any.
     * @return A pointer to the exception, or nullptr if no exception.
     */
    auto getException() -> std::exception_ptr {
        try {
            future_.get();
        } catch (...) {
            return std::current_exception();
        }
        return nullptr;
    }

    /**
     * @brief Retries the operation associated with the future.
     * @tparam F The type of the function to call.
     * @param func The function to call when retrying.
     * @param max_retries The maximum number of retries.
     * @return An EnhancedFuture for the result of the function.
     */
    template <typename F>
    auto retry(F &&func, int max_retries) {
        using ResultType = std::invoke_result_t<F, T>;
        return EnhancedFuture<ResultType>(
            std::async(std::launch::async, [fut = future_,
                                            func = std::forward<F>(func),
                                            max_retries]() mutable {
                for (int attempt = 0; attempt < max_retries; ++attempt) {
                    if (fut.valid()) {
                        try {
                            return func(fut.get());
                        } catch (const std::exception &e) {
                            if (attempt == max_retries - 1) {
                                throw;
                            }
                        }
                    } else {
                        THROW_UNLAWFUL_OPERATION(
                            "Future is invalid or cancelled");
                    }
                }
            }).share());
    }

    auto isReady() -> bool {
        return future_.wait_for(std::chrono::milliseconds(0)) ==
               std::future_status::ready;
    }

    auto get() -> T { return future_.get(); }

protected:
    std::shared_future<T> future_;  ///< The underlying shared future.
    std::vector<std::function<void(T)>>
        callbacks_;   ///< List of callbacks to be called on completion.
    bool cancelled_;  ///< Flag indicating if the future has been cancelled.
};

/**
 * @class EnhancedFuture<void>
 * @brief Specialization of the EnhancedFuture class for void type.
 */
template <>
class EnhancedFuture<void> {
public:
    /**
     * @brief Constructs an EnhancedFuture from a shared future.
     * @param fut The shared future to wrap.
     */
    explicit EnhancedFuture(std::shared_future<void> &&fut)
        : future_(std::move(fut)), cancelled_(false) {}

    explicit EnhancedFuture(const std::shared_future<void> &fut)
        : future_(fut), cancelled_(false) {}

    /**
     * @brief Chains another operation to be called after the future is done.
     * @tparam F The type of the function to call.
     * @param func The function to call when the future is done.
     * @return An EnhancedFuture for the result of the function.
     */
    template <typename F>
    auto then(F &&func) {
        using ResultType = std::invoke_result_t<F>;
        return EnhancedFuture<ResultType>(
            std::async(std::launch::async, [fut = future_,
                                            func = std::forward<F>(
                                                func)]() mutable {
                if (fut.valid()) {
                    fut.get();
                    return func();
                }
                THROW_UNLAWFUL_OPERATION("Future is invalid or cancelled");
            }).share());
    }

    /**
     * @brief Waits for the future with a timeout and auto-cancels if not ready.
     * @param timeout The timeout duration.
     * @return True if the future is ready, false otherwise.
     */
    auto waitFor(std::chrono::milliseconds timeout) -> bool {
        if (future_.wait_for(timeout) == std::future_status::ready &&
            !cancelled_) {
            future_.get();
            return true;
        }
        cancel();
        return false;
    }

    /**
     * @brief Checks if the future is done.
     * @return True if the future is done, false otherwise.
     */
    [[nodiscard]] auto isDone() const -> bool {
        return future_.wait_for(std::chrono::milliseconds(0)) ==
               std::future_status::ready;
    }

    /**
     * @brief Sets a completion callback to be called when the future is done.
     * @tparam F The type of the callback function.
     * @param func The callback function to add.
     */
    template <typename F>
    void onComplete(F &&func) {
        if (!cancelled_) {
            callbacks_.emplace_back(std::forward<F>(func));
            std::async(std::launch::async, [this]() {
                try {
                    if (future_.valid()) {
                        future_.get();
                        for (auto &callback : callbacks_) {
                            callback();
                        }
                    }
                } catch (const std::exception &e) {
                }
            }).get();
        }
    }

    /**
     * @brief Waits synchronously for the future to complete.
     * @throws InvalidFutureException if the future is cancelled.
     */
    void wait() {
        if (cancelled_) {
            THROW_OBJ_NOT_EXIST("Future has been cancelled");
        }
        future_.get();
    }

    /**
     * @brief Cancels the future.
     */
    void cancel() { cancelled_ = true; }

    /**
     * @brief Checks if the future has been cancelled.
     * @return True if the future has been cancelled, false otherwise.
     */
    [[nodiscard]] auto isCancelled() const -> bool { return cancelled_; }

    /**
     * @brief Gets the exception associated with the future, if any.
     * @return A pointer to the exception, or nullptr if no exception.
     */
    auto getException() -> std::exception_ptr {
        try {
            future_.get();
        } catch (...) {
            return std::current_exception();
        }
        return nullptr;
    }

    auto isReady() -> bool {
        return future_.wait_for(std::chrono::milliseconds(0)) ==
               std::future_status::ready;
    }

    auto get() -> void { future_.get(); }

protected:
    std::shared_future<void> future_;  ///< The underlying shared future.
    std::vector<std::function<void()>>
        callbacks_;  ///< List of callbacks to be called on completion.
    std::atomic<bool>
        cancelled_;  ///< Flag indicating if the future has been cancelled.
};

/**
 * @brief Helper function to create an EnhancedFuture.
 * @tparam F The type of the function to call.
 * @tparam Args The types of the arguments to pass to the function.
 * @param f The function to call.
 * @param args The arguments to pass to the function.
 * @return An EnhancedFuture for the result of the function.
 */
template <typename F, typename... Args>
auto makeEnhancedFuture(F &&f, Args &&...args) {
    using result_type = std::invoke_result_t<F, Args...>;
    return EnhancedFuture<result_type>(std::async(std::launch::async,
                                                  std::forward<F>(f),
                                                  std::forward<Args>(args)...)
                                           .share());
}

/**
 * @brief Helper function to get a future for a range of futures.
 * @tparam InputIt The type of the input iterator.
 * @param first The beginning of the range.
 * @param last The end of the range.
 * @param timeout An optional timeout duration.
 * @return A future containing a vector of the results of the input futures.
 */
template <typename InputIt>
auto whenAll(InputIt first, InputIt last,
             std::optional<std::chrono::milliseconds> timeout = std::nullopt)
    -> std::future<
        std::vector<typename std::iterator_traits<InputIt>::value_type>> {
    using FutureType = typename std::iterator_traits<InputIt>::value_type;
    using ResultType = std::vector<FutureType>;

    std::promise<ResultType> promise;
    std::future<ResultType> resultFuture = promise.get_future();

    // Launch an async task to wait for all the futures
    auto asyncTask = std::async([promise = std::move(promise), first, last,
                                 timeout]() mutable {
        ResultType results;
        try {
            for (auto it = first; it != last; ++it) {
                if (timeout) {
                    // Check each future with timeout (if specified)
                    if (it->wait_for(*timeout) == std::future_status::timeout) {
                        THROW_INVALID_ARGUMENT(
                            "Timeout while waiting for a future.");
                    }
                }
                results.push_back(std::move(*it));
            }
            promise.set_value(std::move(results));
        } catch (const std::exception &e) {
            promise.set_exception(
                std::current_exception());  // Pass the exception to the future
        }
    });

    // Optionally, store the future or use it if needed
    asyncTask.wait();  // Wait for the async task to finish

    return resultFuture;
}

/**
 * @brief Helper to get the return type of a future.
 * @tparam T The type of the future.
 */
template <typename T>
using future_value_t = decltype(std::declval<T>().get());

/**
 * @brief Helper function for a variadic template version (when_all for futures
 * as arguments).
 * @tparam Futures The types of the futures.
 * @param futures The futures to wait for.
 * @return A future containing a tuple of the results of the input futures.
 */
template <typename... Futures>
auto whenAll(Futures &&...futures)
    -> std::future<std::tuple<future_value_t<Futures>...>> {
    std::promise<std::tuple<future_value_t<Futures>...>> promise;
    std::future<std::tuple<future_value_t<Futures>...>> resultFuture =
        promise.get_future();

    // Use async to wait for all futures and gather results
    auto asyncTask =
        std::async([promise = std::move(promise),
                    futures = std::make_tuple(
                        std::forward<Futures>(futures)...)]() mutable {
            try {
                auto results = std::apply(
                    [](auto &&...fs) {
                        return std::make_tuple(
                            fs.get()...);  // Wait for each future and collect
                                           // the results
                    },
                    futures);
                promise.set_value(std::move(results));
            } catch (const std::exception &e) {
                promise.set_exception(std::current_exception());
            }
        });

    asyncTask.wait();  // Wait for the async task to finish

    return resultFuture;
}

}  // namespace atom::async

#endif  // ATOM_ASYNC_FUTURE_HPP
