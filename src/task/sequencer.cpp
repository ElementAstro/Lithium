/**
 * @file sequencer.cpp
 * @brief Definition of classes for managing and executing task sequences.
 *
 * This file defines the `Target` and `ExposureSequence` classes for managing
 * and executing sequences of tasks. The `Target` class represents a unit that
 * can hold and execute tasks with a configurable delay and priority. The
 * `ExposureSequence` class manages a collection of `Target` objects and
 * coordinates their execution, allowing for task sequences to be executed in
 * parallel or serially with options for pausing, resuming, and stopping.
 *
 * Key features:
 * - `Target` class: Manages individual tasks, delay after execution, and
 *   priority. Supports enabling/disabling and task execution.
 * - `ExposureSequence` class: Manages multiple `Target` instances, supports
 *   adding, removing, and modifying targets, and coordinates their execution
 *   in a controlled manner.
 *
 * @date 2023-04-03
 * @author Max Qian <lightapt.com>
 * @copyright Copyright (C) 2023-2024 Max Qian
 */

#include "sequencer.hpp"
#include "task.hpp"

#include "atom/log/loguru.hpp"
#include "type/expected.hpp"

Target::Target(std::string name, std::chrono::seconds delayAfterTarget,
               int priority)
    : name_(std::move(name)),
      delayAfterTarget_(delayAfterTarget),
      priority_(priority),
      enabled_(true) {}

void Target::addTask(std::shared_ptr<Task> task) {
    tasks_.emplace_back(std::move(task));
}

void Target::setDelayAfterTarget(std::chrono::seconds delay) {
    delayAfterTarget_ = delay;
}

void Target::setPriority(int p) { priority_ = p; }

int Target::getPriority() const { return priority_; }

void Target::enable() { enabled_ = true; }

void Target::disable() { enabled_ = false; }

auto Target::isEnabled() const -> bool { return enabled_; }

auto Target::execute(std::stop_token stopToken, std::atomic_flag& pauseFlag,
                     std::condition_variable_any& cv, std::shared_mutex& mtx)
    -> atom::type::expected<void, std::string> {
    if (!enabled_) {
        LOG_F(WARNING, "Target {} is disabled.", name_);
        return atom::type::make_unexpected("Target is disabled");
    }

    LOG_F(INFO, "Starting target: {}", name_);
    for (const auto& task : tasks_) {
        if (stopToken.stop_requested()) {
            return {};
        }

        std::shared_lock lock(mtx);
        cv.wait(lock, [&] {
            return !pauseFlag.test() || stopToken.stop_requested();
        });

        if (stopToken.stop_requested()) {
            return {};
        }

        try {
            task->start();
            task->run();
            if (task->isTimeout()) {
                LOG_F(ERROR, "Task {} timed out.", task->getName());
                task->fail(std::runtime_error("Timeout"));
            }
        } catch (const std::exception& ex) {
            LOG_F(ERROR, "Task {} failed with exception: {}", task->getName(),
                  ex.what());
            return atom::type::make_unexpected("Task execution failed");
        }
    }
    LOG_F(INFO, "Completed target: {}", name_);
    std::this_thread::sleep_for(delayAfterTarget_);
    return {};
}

std::string Target::getName() const { return name_; }

ExposureSequence::ExposureSequence() : sequenceThread_(nullptr) {}

ExposureSequence::~ExposureSequence() {
    stop();
    if (sequenceThread_ && sequenceThread_->joinable()) {
        sequenceThread_->join();
    }
}

void ExposureSequence::addTarget(Target target) {
    std::unique_lock lock(mutex_);
    targets_.emplace_back(std::make_shared<Target>(std::move(target)));
    LOG_F(INFO, "Added target: {}", target.getName());
}

void ExposureSequence::removeTarget(size_t index) {
    std::unique_lock lock(mutex_);
    if (index < targets_.size()) {
        targets_.erase(targets_.begin() + index);
        LOG_F(INFO, "Removed target at index {}", index);
    } else {
        LOG_F(ERROR, "Target index out of range for removal.");
    }
}

void ExposureSequence::modifyTarget(
    size_t index, std::optional<std::chrono::seconds> newDelay,
    std::optional<int> newPriority) {
    std::unique_lock lock(mutex_);
    if (index < targets_.size()) {
        if (newDelay) {
            targets_[index]->setDelayAfterTarget(*newDelay);
        }
        if (newPriority) {
            targets_[index]->setPriority(*newPriority);
        }
        LOG_F(INFO, "Modified target at index {}", index);
    } else {
        LOG_F(ERROR, "Target index out of range for modification.");
    }
}

void ExposureSequence::enableTarget(size_t index) {
    std::unique_lock lock(mutex_);
    if (index < targets_.size()) {
        targets_[index]->enable();
        LOG_F(INFO, "Enabled target at index {}", index);
    } else {
        LOG_F(ERROR, "Target index out of range for enabling.");
    }
}

void ExposureSequence::disableTarget(size_t index) {
    std::unique_lock lock(mutex_);
    if (index < targets_.size()) {
        targets_[index]->disable();
        LOG_F(INFO, "Disabled target at index {}", index);
    } else {
        LOG_F(ERROR, "Target index out of range for disabling.");
    }
}

void ExposureSequence::executeAll() {
    stopFlag_.clear();
    pauseFlag_.clear();
    if (sequenceThread_ && sequenceThread_->joinable()) {
        sequenceThread_->join();
    }
    sequenceThread_ = std::make_unique<std::jthread>(
        &ExposureSequence::executeSequence, this);
}

void ExposureSequence::stop() {
    if (sequenceThread_) {
        sequenceThread_->request_stop();
        cv_.notify_all();
        LOG_F(INFO, "Stopping all tasks.");
    }
}

void ExposureSequence::pause() {
    pauseFlag_.test_and_set();
    LOG_F(INFO, "Pausing all tasks.");
}

void ExposureSequence::resume() {
    pauseFlag_.clear();
    cv_.notify_all();
    LOG_F(INFO, "Resuming all tasks.");
}

void ExposureSequence::executeSequence(std::stop_token stopToken) {
    for (const auto& target : targets_) {
        if (stopToken.stop_requested()) {
            return;
        }
        if (target->isEnabled()) {
            target->execute(stopToken, pauseFlag_, cv_, mutex_)
                .value_or([&](std::string err) {
                    LOG_F(ERROR, "Failed to execute target: {}", err);
                });
        }
    }
}
