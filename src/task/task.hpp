#ifndef LITHIUM_TASK_TASK_HPP
#define LITHIUM_TASK_TASK_HPP

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>
#include "atom/type/json.hpp"

using json = nlohmann::json;

/**
 * @class Event
 * @brief A base class for events.
 */
class Event {
public:
    virtual ~Event() = default;
};

/**
 * @class State
 * @brief A base class for states in the state machine.
 */
class State {
public:
    virtual ~State() = default;

    /**
     * @brief Called when entering the state.
     */
    virtual void onEnter() {}

    /**
     * @brief Called when exiting the state.
     */
    virtual void onExit() {}

    /**
     * @brief Handle an incoming event.
     * @param event The event to handle.
     */
    virtual void handleEvent(std::shared_ptr<Event> event) = 0;

    /**
     * @brief Get the name of the state.
     * @return The name of the state.
     */
    virtual std::string getName() const = 0;
};

/**
 * @class StateMachine
 * @brief A state machine for managing state transitions.
 */
class StateMachine {
public:
    using StatePtr = std::shared_ptr<State>;

    /**
     * @brief Add a state to the state machine.
     * @param state The state to add.
     */
    void addState(StatePtr state);

    /**
     * @brief Set the initial state of the state machine.
     * @param stateName The name of the initial state.
     */
    void setInitialState(const std::string& stateName);

    /**
     * @brief Transition to a new state.
     * @param stateName The name of the state to transition to.
     */
    void transitionTo(const std::string& stateName);

    /**
     * @brief Handle an incoming event.
     * @param event The event to handle.
     */
    void handleEvent(std::shared_ptr<Event> event);

    /**
     * @brief Get the current state.
     * @return The current state.
     */
    StatePtr getCurrentState() const;

protected:
    std::unordered_map<std::string, StatePtr>
        states; /**< Map of state names to state objects. */
    StatePtr currentState = nullptr; /**< The current state. */
};

class Task;

/**
 * @class TaskEvent
 * @brief An event specific to tasks.
 */
class TaskEvent : public Event {
public:
    /**
     * @enum Type
     * @brief The type of the task event.
     */
    enum Type { Start, Complete, Fail };

    /**
     * @brief Construct a TaskEvent with a specific type.
     * @param type The type of the event.
     */
    explicit TaskEvent(Type type);

    /**
     * @brief Get the type of the event.
     * @return The type of the event.
     */
    Type getType() const;

private:
    Type eventType; /**< The type of the event. */
};

/**
 * @class TaskState
 * @brief A base class for task states.
 */
class TaskState : public State {
public:
    /**
     * @brief Construct a TaskState with a reference to a task.
     * @param task The task associated with this state.
     */
    explicit TaskState(Task& task);

protected:
    Task& task; /**< The task associated with this state. */
};

/**
 * @class PendingState
 * @brief A state representing a pending task.
 */
class PendingState : public TaskState {
public:
    using TaskState::TaskState;

    /**
     * @brief Get the name of the state.
     * @return The name of the state.
     */
    std::string getName() const override;

    /**
     * @brief Handle an incoming event.
     * @param event The event to handle.
     */
    void handleEvent(std::shared_ptr<Event> event) override;

    /**
     * @brief Called when entering the state.
     */
    void onEnter() override;

    /**
     * @brief Called when exiting the state.
     */
    void onExit() override;
};

/**
 * @class RunningState
 * @brief A state representing a running task.
 */
class RunningState : public TaskState {
public:
    using TaskState::TaskState;

    /**
     * @brief Get the name of the state.
     * @return The name of the state.
     */
    std::string getName() const override;

    /**
     * @brief Handle an incoming event.
     * @param event The event to handle.
     */
    void handleEvent(std::shared_ptr<Event> event) override;

    /**
     * @brief Called when entering the state.
     */
    void onEnter() override;

    /**
     * @brief Called when exiting the state.
     */
    void onExit() override;
};

/**
 * @class CompletedState
 * @brief A state representing a completed task.
 */
class CompletedState : public TaskState {
public:
    using TaskState::TaskState;

    /**
     * @brief Get the name of the state.
     * @return The name of the state.
     */
    std::string getName() const override;

    /**
     * @brief Handle an incoming event.
     * @param event The event to handle.
     */
    void handleEvent([[maybe_unused]] std::shared_ptr<Event> event) override {}

    /**
     * @brief Called when entering the state.
     */
    void onEnter() override;

    /**
     * @brief Called when exiting the state.
     */
    void onExit() override;
};

/**
 * @class FailedState
 * @brief A state representing a failed task.
 */
class FailedState : public TaskState {
public:
    using TaskState::TaskState;

    /**
     * @brief Get the name of the state.
     * @return The name of the state.
     */
    std::string getName() const override;

    /**
     * @brief Handle an incoming event.
     * @param event The event to handle.
     */
    void handleEvent([[maybe_unused]] std::shared_ptr<Event> event) override {}

    /**
     * @brief Called when entering the state.
     */
    void onEnter() override;

    /**
     * @brief Called when exiting the state.
     */
    void onExit() override;
};

/**
 * @class TaskCanceledException
 * @brief An exception thrown when a task is canceled.
 */
class TaskCanceledException : public std::exception {
public:
    /**
     * @brief Get the description of the exception.
     * @return The description of the exception.
     */
    const char* what() const noexcept override;
};

/**
 * @class Task
 * @brief Represents a task that can be executed and managed.
 */
class Task {
public:
    /**
     * @enum Status
     * @brief The status of the task.
     */
    enum class Status { Pending, Running, Completed, Failed };

    /**
     * @brief Construct a task.
     * @param name The name of the task.
     * @param params The parameters for the task.
     * @param function The function to execute for the task.
     * @param onTerminate The function to call on termination, if any.
     */
    Task(const std::string& name, const json& params,
         std::function<json(const json&)> function,
         std::optional<std::function<void(const std::exception&)>> onTerminate =
             std::nullopt);

    /**
     * @brief Start the task.
     */
    void start();

    /**
     * @brief Run the task.
     */
    void run();

    /**
     * @brief Complete the task.
     */
    void complete();

    /**
     * @brief Fail the task.
     * @param e The exception that caused the failure.
     */
    void fail(const std::exception& e);

    /**
     * @brief Get the name of the task.
     * @return The name of the task.
     */
    const std::string& getName() const;

    /**
     * @brief Get the parameters of the task.
     * @return The parameters of the task.
     */
    const json& getParams() const;

    /**
     * @brief Get the result of the task, if any.
     * @return The result of the task.
     */
    const std::optional<json>& getResult() const;

    /**
     * @brief Get the status of the task.
     * @return The status of the task.
     */
    Status getStatus() const;

    /**
     * @brief Set the status of the task.
     * @param status The status to set.
     */
    void setStatus(Status status);

    /**
     * @brief Get the state machine of the task.
     * @return The state machine of the task.
     */
    StateMachine& getStateMachine();

    using CustomFunction = std::function<void(Task&)>;

    /**
     * @brief Register a custom function to be executed for a specific status.
     * @param status The status for which the function should be executed.
     * @param function The function to execute.
     */
    void registerCustomFunction(Status status, CustomFunction function);

    /**
     * @brief Execute custom functions for a specific status.
     * @param status The status for which to execute custom functions.
     */
    void executeCustomFunctions(Status status);

    /**
     * @brief Cancel the task.
     */
    void cancel();

    /**
     * @brief Set the progress of the task.
     * @param progress The progress to set.
     */
    void setProgress(double progress);

    /**
     * @brief Get the progress of the task.
     * @return The progress of the task.
     */
    double getProgress() const;

    /**
     * @brief Set the timeout for the task.
     * @param timeout The timeout duration.
     */
    void setTimeout(std::chrono::milliseconds timeout);

    /**
     * @brief Check if the task has timed out.
     * @return True if the task has timed out, false otherwise.
     */
    bool isTimeout() const;

private:
    std::string name; /**< The name of the task. */
    json params;      /**< The parameters for the task. */
    std::function<json(const json&)>
        function; /**< The function to execute for the task. */
    std::optional<std::function<void(const std::exception&)>>
        onTerminate; /**< The function to call on termination, if any. */
    std::unordered_map<Status, std::vector<CustomFunction>>
        customFunctions; /**< Custom functions to execute for specific statuses.
                          */
    Status status = Status::Pending; /**< The status of the task. */
    std::optional<json> result;      /**< The result of the task, if any. */
    StateMachine statusMachine;      /**< The state machine of the task. */
    double progress = 0.0;           /**< The progress of the task. */
    std::optional<std::chrono::milliseconds>
        timeout; /**< The timeout duration for the task, if any. */
    std::chrono::steady_clock::time_point
        startTime; /**< The start time of the task. */
};

#endif  // LITHIUM_TASK_TASK_HPP