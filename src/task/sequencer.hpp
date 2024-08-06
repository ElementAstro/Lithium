#ifndef LITHIUM_TASK_SEQUENCER_HPP
#define LITHIUM_TASK_SEQUENCER_HPP

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "task.hpp"

// Forward declaration of StateMachine class
class StateMachine;

// Forward declaration of TaskEvent class
class TaskEvent;

class Target {
public:
    explicit Target(std::string name, int delayAfterTarget = 0,
                    int priority = 0);

    void addTask(std::shared_ptr<Task> task);

    void setDelayAfterTarget(int delay);

    void setPriority(int p);

    [[nodiscard]] auto getPriority() const -> int;

    void enable();

    void disable();

    [[nodiscard]] auto isEnabled() const -> bool;

    void execute(std::atomic<bool>& stopFlag, std::atomic<bool>& pauseFlag,
                 std::condition_variable& cv, std::mutex& mtx);

    [[nodiscard]] auto getName() const -> std::string;

private:
    std::string name_;
    std::vector<std::shared_ptr<Task>> tasks_;
    int delayAfterTarget_;
    int priority_;
    bool enabled_ = true;
};

class ExposureSequence {
public:
    ExposureSequence();

    ~ExposureSequence();

    void addTarget(Target target);

    void removeTarget(size_t index);

    void modifyTarget(size_t index, std::optional<int> newDelay = std::nullopt,
                      std::optional<int> newPriority = std::nullopt);

    void enableTarget(size_t index);

    void disableTarget(size_t index);

    void executeAll();

    void stop();

    void pause();

    void resume();

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::shared_ptr<Target>> targets_;
    std::atomic<bool> stopFlag_;
    std::atomic<bool> pauseFlag_;
    std::unique_ptr<std::thread> sequenceThread_;

    void executeSequence();
};

#endif