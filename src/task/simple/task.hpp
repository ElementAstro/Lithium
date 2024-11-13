/**
 * @file task.hpp
 * @brief Defines the Task class for executing tasks with optional timeout.
 */

#ifndef TASK_HPP
#define TASK_HPP

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include "atom/type/json.hpp"

namespace lithium::sequencer {
using json = nlohmann::json;
/**
 * @enum TaskStatus
 * @brief Represents the status of a task.
 */
enum class TaskStatus {
    Pending,     ///< Task is pending and has not started yet.
    InProgress,  ///< Task is currently in progress.
    Completed,   ///< Task has completed successfully.
    Failed       ///< Task has failed.
};

/**
 * @class Task
 * @brief Represents a task that can be executed with an optional timeout.
 */
class Task {
public:
    /**
     * @brief Constructs a Task with a given name and action.
     * @param name The name of the task.
     * @param action The action to be performed by the task.
     */
    Task(std::string name, std::function<void(const json&)> action);

    /**
     * @brief Executes the task with the given parameters.
     * @param params The parameters to be passed to the task action.
     */
    void execute(const json& params);

    /**
     * @brief Sets the timeout for the task.
     * @param timeout The timeout duration in seconds.
     */
    void setTimeout(std::chrono::seconds timeout);

    /**
     * @brief Gets the name of the task.
     * @return The name of the task.
     */
    [[nodiscard]] auto getName() const -> const std::string&;

    /**
     * @brief Gets the UUID of the task.
     * @return The UUID of the task.
     */
    [[nodiscard]] auto getUUID() const -> const std::string&;

    /**
     * @brief Gets the current status of the task.
     * @return The current status of the task.
     */
    [[nodiscard]] auto getStatus() const -> TaskStatus;

    /**
     * @brief Gets the error message if the task has failed.
     * @return An optional string containing the error message if the task has
     * failed, otherwise std::nullopt.
     */
    [[nodiscard]] auto getError() const -> std::optional<std::string>;

private:
    std::string name_;  ///< The name of the task.
    std::string uuid_;  ///< The unique identifier of the task.
    std::function<void(const json&)>
        action_;  ///< The action to be performed by the task.
    std::chrono::seconds timeout_{0};  ///< The timeout duration for the task.
    TaskStatus status_{
        TaskStatus::Pending};  ///< The current status of the task.
    std::optional<std::string>
        error_;  ///< The error message if the task has failed.
};

/**
 * @brief Base class for task creation using static polymorphism.
 */
template <typename Derived>
class TaskCreator {
public:
    static auto createTask() -> std::unique_ptr<Task> {
        return std::make_unique<Task>(Derived::taskName(), Derived::execute);
    }
};

}  // namespace lithium::sequencer

#endif  // TASK_HPP