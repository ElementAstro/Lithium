#ifndef ATOM_ASYNC_PACKAGED_TASK_HPP
#define ATOM_ASYNC_PACKAGED_TASK_HPP

#include <atomic>
#include <functional>
#include <future>
#include <vector>

#include "atom/async/future.hpp"

namespace atom::async {

/**
 * @class InvalidPackagedTaskException
 * @brief Exception thrown when an invalid packaged task is encountered.
 */
class InvalidPackagedTaskException : public atom::error::RuntimeError {
public:
    using atom::error::RuntimeError::RuntimeError;
};

/**
 * @def THROW_INVALID_PACKAGED_TASK_EXCEPTION
 * @brief Macro to throw an InvalidPackagedTaskException with file, line, and
 * function information.
 */
#define THROW_INVALID_PACKAGED_TASK_EXCEPTION(...)                     \
    throw InvalidPackagedTaskException(ATOM_FILE_NAME, ATOM_FILE_LINE, \
                                       ATOM_FUNC_NAME, __VA_ARGS__);

/**
 * @def THROW_NESTED_INVALID_PACKAGED_TASK_EXCEPTION
 * @brief Macro to rethrow a nested InvalidPackagedTaskException with file,
 * line, and function information.
 */
#define THROW_NESTED_INVALID_PACKAGED_TASK_EXCEPTION(...) \
    InvalidPackagedTaskException::rethrowNested(          \
        ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME,   \
        "Invalid packaged task: " __VA_ARGS__);

/**
 * @class EnhancedPackagedTask
 * @brief A template class that extends the standard packaged task with
 * additional features.
 * @tparam ResultType The type of the result that the task will produce.
 * @tparam Args The types of the arguments that the task will accept.
 */
template <typename ResultType, typename... Args>
class EnhancedPackagedTask {
public:
    using TaskType = std::function<ResultType(Args...)>;

    /**
     * @brief Constructs an EnhancedPackagedTask with the given task.
     * @param task The task to be executed.
     */
    explicit EnhancedPackagedTask(TaskType task)
        : task_(std::move(task)), cancelled_(false) {
        promise_ = std::promise<ResultType>();
        future_ = promise_.get_future().share();
    }

    /**
     * @brief Gets the enhanced future associated with this task.
     * @return An EnhancedFuture object.
     */
    EnhancedFuture<ResultType> getEnhancedFuture() {
        return EnhancedFuture<ResultType>(std::move(future_));
    }

    /**
     * @brief Executes the task with the given arguments.
     * @param args The arguments to pass to the task.
     */
    void operator()(Args... args) {
        if (cancelled_) {
            promise_.set_exception(std::make_exception_ptr(
                std::runtime_error("Task has been cancelled")));
            return;
        }

        try {
            if (task_) {
                ResultType result = task_(std::forward<Args>(args)...);
                promise_.set_value(result);
                runCallbacks(result);
            }
        } catch (...) {
            promise_.set_exception(std::current_exception());
        }
    }

    /**
     * @brief Adds a callback to be called upon task completion.
     * @tparam F The type of the callback function.
     * @param func The callback function to add.
     */
    template <typename F>
    void onComplete(F &&func) {
        callbacks_.emplace_back(std::forward<F>(func));
    }

    /**
     * @brief Cancels the task.
     */
    void cancel() { cancelled_ = true; }

    /**
     * @brief Checks if the task is cancelled.
     * @return True if the task is cancelled, false otherwise.
     */
    [[nodiscard]] bool isCancelled() const { return cancelled_; }

protected:
    TaskType task_;  ///< The task to be executed.
    std::promise<ResultType>
        promise_;  ///< The promise associated with the task.
    std::shared_future<ResultType>
        future_;  ///< The shared future associated with the task.
    std::vector<std::function<void(ResultType)>>
        callbacks_;  ///< List of callbacks to be called on completion.
    std::atomic<bool>
        cancelled_;  ///< Flag indicating if the task has been cancelled.

private:
    /**
     * @brief Runs all the registered callbacks with the given result.
     * @param result The result to pass to the callbacks.
     */
    void runCallbacks(ResultType result) {
        for (auto &callback : callbacks_) {
            callback(result);
        }
    }
};

/**
 * @class EnhancedPackagedTask<void, Args...>
 * @brief Specialization of the EnhancedPackagedTask class for void result type.
 * @tparam Args The types of the arguments that the task will accept.
 */
template <typename... Args>
class EnhancedPackagedTask<void, Args...> {
public:
    using TaskType = std::function<void(Args...)>;

    /**
     * @brief Constructs an EnhancedPackagedTask with the given task.
     * @param task The task to be executed.
     */
    explicit EnhancedPackagedTask(TaskType task)
        : task_(std::move(task)), cancelled_(false) {
        promise_ = std::promise<void>();
        future_ = promise_.get_future().share();
    }

    /**
     * @brief Gets the enhanced future associated with this task.
     * @return An EnhancedFuture object.
     */
    EnhancedFuture<void> getEnhancedFuture() {
        return EnhancedFuture<void>(std::move(future_));
    }

    /**
     * @brief Executes the task with the given arguments.
     * @param args The arguments to pass to the task.
     */
    void operator()(Args... args) {
        if (cancelled_) {
            promise_.set_exception(std::make_exception_ptr(
                std::runtime_error("Task has been cancelled")));
            return;
        }

        try {
            if (task_) {
                task_(std::forward<Args>(args)...);
                promise_.set_value();
                runCallbacks();
            }
        } catch (...) {
            promise_.set_exception(std::current_exception());
        }
    }

    /**
     * @brief Adds a callback to be called upon task completion.
     * @tparam F The type of the callback function.
     * @param func The callback function to add.
     */
    template <typename F>
    void onComplete(F &&func) {
        callbacks_.emplace_back(std::forward<F>(func));
    }

    /**
     * @brief Cancels the task.
     */
    void cancel() { cancelled_ = true; }

    /**
     * @brief Checks if the task is cancelled.
     * @return True if the task is cancelled, false otherwise.
     */
    [[nodiscard]] bool isCancelled() const { return cancelled_; }

protected:
    TaskType task_;               ///< The task to be executed.
    std::promise<void> promise_;  ///< The promise associated with the task.
    std::shared_future<void>
        future_;  ///< The shared future associated with the task.
    std::vector<std::function<void()>>
        callbacks_;  ///< List of callbacks to be called on completion.
    std::atomic<bool>
        cancelled_;  ///< Flag indicating if the task has been cancelled.

private:
    /**
     * @brief Runs all the registered callbacks.
     */
    void runCallbacks() {
        for (auto &callback : callbacks_) {
            callback();
        }
    }
};

}  // namespace atom::async

#endif  // ATOM_ASYNC_PACKAGED_TASK_HPP