// target.hpp
#ifndef LITHIUM_TARGET_HPP
#define LITHIUM_TARGET_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#include "atom/async/safetype.hpp"
#include "task.hpp"

namespace lithium::sequencer {

/**
 * @enum TargetStatus
 * @brief Represents the status of a target.
 */
enum class TargetStatus {
    Pending,     ///< Target is pending and has not started yet.
    InProgress,  ///< Target is currently in progress.
    Completed,   ///< Target has completed successfully.
    Failed,      ///< Target has failed.
    Skipped      ///< Target has been skipped.
};

/**
 * @brief Callback function type definitions.
 */
using TargetStartCallback = std::function<void(const std::string&)>;
using TargetEndCallback = std::function<void(const std::string&, TargetStatus)>;
using TargetErrorCallback =
    std::function<void(const std::string&, const std::exception&)>;

class Target;

/**
 * @brief Target modifier type definition.
 */
using TargetModifier = std::function<void(Target&)>;

/**
 * @class Target
 * @brief Represents a target that can execute a series of tasks with optional
 * retries and cooldown periods.
 */
class Target {
public:
    /**
     * @brief Constructs a Target with a given name, cooldown period, and
     * maximum retries.
     * @param name The name of the target.
     * @param cooldown The cooldown period between task executions.
     * @param maxRetries The maximum number of retries for each task.
     */
    Target(std::string name,
           std::chrono::seconds cooldown = std::chrono::seconds{0},
           int maxRetries = 0);

    // Disable copy constructor and assignment operator
    Target(const Target&) = delete;
    Target& operator=(const Target&) = delete;

    /**
     * @brief Adds a task to the target.
     * @param task The task to be added.
     */
    void addTask(std::unique_ptr<Task> task);

    /**
     * @brief Sets the cooldown period for the target.
     * @param cooldown The cooldown period in seconds.
     */
    void setCooldown(std::chrono::seconds cooldown);

    /**
     * @brief Enables or disables the target.
     * @param enabled True to enable, false to disable.
     */
    void setEnabled(bool enabled);

    /**
     * @brief Sets the maximum number of retries for each task.
     * @param retries The maximum number of retries.
     */
    void setMaxRetries(int retries);

    /**
     * @brief Sets the status of the target.
     * @param status The status to be set.
     */
    void setStatus(TargetStatus status);

    /**
     * @brief Sets the callback function to be called when the target starts.
     * @param callback The callback function.
     */
    void setOnStart(TargetStartCallback callback);

    /**
     * @brief Sets the callback function to be called when the target ends.
     * @param callback The callback function.
     */
    void setOnEnd(TargetEndCallback callback);

    /**
     * @brief Sets the callback function to be called when an error occurs.
     * @param callback The callback function.
     */
    void setOnError(TargetErrorCallback callback);

    /**
     * @brief Gets the name of the target.
     * @return The name of the target.
     */
    [[nodiscard]] const std::string& getName() const;

    /**
     * @brief Gets the UUID of the target.
     * @return The UUID of the target.
     */
    [[nodiscard]] const std::string& getUUID() const;

    /**
     * @brief Gets the current status of the target.
     * @return The current status of the target.
     */
    [[nodiscard]] TargetStatus getStatus() const;

    /**
     * @brief Checks if the target is enabled.
     * @return True if the target is enabled, false otherwise.
     */
    [[nodiscard]] bool isEnabled() const;

    /**
     * @brief Gets the progress of the target as a percentage.
     * @return The progress percentage.
     */
    [[nodiscard]] double getProgress() const;

    /**
     * @brief Executes the target.
     */
    virtual void execute();

     /**
     * @brief Loads tasks from a JSON array.
     * @param tasksJson The JSON array containing task definitions.
     */
    void loadTasksFromJson(const json& tasksJson);

private:
    std::string name_;  ///< The name of the target.
    std::string uuid_;  ///< The unique identifier of the target.
    std::vector<std::unique_ptr<Task>>
        tasks_;  ///< The list of tasks to be executed by the target.
    std::chrono::seconds
        cooldown_;        ///< The cooldown period between task executions.
    bool enabled_{true};  ///< Indicates whether the target is enabled.
    std::atomic<TargetStatus> status_{
        TargetStatus::Pending};  ///< The current status of the target.
    std::shared_mutex
        mutex_;  ///< Mutex for thread-safe access to target properties.

    // Progress tracking
    std::atomic<size_t> completedTasks_{0};  ///< The number of completed tasks.
    size_t totalTasks_ = 0;                  ///< The total number of tasks.

    // Callback functions
    TargetStartCallback
        onStart_;  ///< Callback function to be called when the target starts.
    TargetEndCallback
        onEnd_;  ///< Callback function to be called when the target ends.
    TargetErrorCallback
        onError_;  ///< Callback function to be called when an error occurs.

    // Retry mechanism
    int maxRetries_;  ///< The maximum number of retries for each task.
    mutable std::shared_mutex callbackMutex_;  ///< Mutex for thread-safe access
                                               ///< to callback functions.

    std::shared_ptr<atom::async::LockFreeHashTable<std::string, json>>
        queue_;  ///< The task queue.

    // Helper methods
    void notifyStart();
    void notifyEnd(TargetStatus status);
    void notifyError(const std::exception& e);
};

}  // namespace lithium::sequencer

#endif  // LITHIUM_TARGET_HPP