#include "target.hpp"

#include "task_camera.hpp"

#include <mutex>
#include <stdexcept>
#include <thread>

#include "config/configor.hpp"

#include "atom/async/message_bus.hpp"
#include "atom/async/safetype.hpp"
#include "atom/error/exception.hpp"
#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/uuid.hpp"

#include "matchit/matchit.h"

#include "utils/constant.hpp"

namespace lithium::sequencer {
class TaskErrorException : public atom::error::RuntimeError {
public:
    using atom::error::RuntimeError::RuntimeError;
};

#define THROW_TASK_ERROR_EXCEPTION(...)                                      \
    throw TaskErrorException(ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, \
                             __VA_ARGS__);

Target::Target(std::string name, std::chrono::seconds cooldown, int maxRetries)
    : name_(std::move(name)),
      uuid_(atom::utils::UUID().toString()),
      cooldown_(cooldown),
      maxRetries_(maxRetries) {
    LOG_F(INFO, "Target created with name: {}, cooldown: {}s, maxRetries: {}",
          name_, cooldown_.count(), maxRetries_);
    if (auto queueOpt =
            GetPtr<atom::async::LockFreeHashTable<std::string, json>>(
                Constants::TASK_QUEUE)) {
        queue_ = queueOpt.value();
    } else {
        THROW_RUNTIME_ERROR("Task queue not found in global shared memory");
    }
}

void Target::addTask(std::unique_ptr<Task> task) {
    if (!task) {
        THROW_INVALID_ARGUMENT("Cannot add a null task");
    }
    std::unique_lock lock(mutex_);
    tasks_.emplace_back(std::move(task));
    totalTasks_ = tasks_.size();
    LOG_F(INFO, "Task added to target: {}, total tasks: {}", name_,
          totalTasks_);
}

void Target::setCooldown(std::chrono::seconds cooldown) {
    std::unique_lock lock(mutex_);
    cooldown_ = cooldown;
    LOG_F(INFO, "Cooldown set to {}s for target: {}", cooldown_.count(), name_);
}

void Target::setEnabled(bool enabled) {
    std::unique_lock lock(mutex_);
    enabled_ = enabled;
    LOG_F(INFO, "Target {} enabled status set to: {}", name_, enabled_);
}

void Target::setMaxRetries(int retries) {
    std::unique_lock lock(mutex_);
    maxRetries_ = retries;
    LOG_F(INFO, "Max retries set to {} for target: {}", retries, name_);
}

void Target::setOnStart(TargetStartCallback callback) {
    std::unique_lock lock(callbackMutex_);
    onStart_ = std::move(callback);
    LOG_F(INFO, "OnStart callback set for target: {}", name_);
}

void Target::setOnEnd(TargetEndCallback callback) {
    std::unique_lock lock(callbackMutex_);
    onEnd_ = std::move(callback);
    LOG_F(INFO, "OnEnd callback set for target: {}", name_);
}

void Target::setOnError(TargetErrorCallback callback) {
    std::unique_lock lock(callbackMutex_);
    onError_ = std::move(callback);
    LOG_F(INFO, "OnError callback set for target: {}", name_);
}

void Target::setStatus(TargetStatus status) {
    std::unique_lock lock(mutex_);
    status_ = status;
    LOG_F(INFO, "Status set to {} for target: {}", static_cast<int>(status),
          name_);
}

const std::string& Target::getName() const { return name_; }

const std::string& Target::getUUID() const { return uuid_; }

TargetStatus Target::getStatus() const { return status_.load(); }

bool Target::isEnabled() const { return enabled_; }

double Target::getProgress() const {
    size_t completed = completedTasks_.load();
    size_t total = totalTasks_;
    if (total == 0) {
        return 100.0;
    }
    return (static_cast<double>(completed) / static_cast<double>(total)) *
           100.0;
}

void Target::notifyStart() {
    TargetStartCallback callbackCopy;
    {
        std::shared_lock lock(callbackMutex_);
        callbackCopy = onStart_;
    }
    if (callbackCopy) {
        try {
            callbackCopy(name_);
            LOG_F(INFO, "OnStart callback executed for target: {}", name_);
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in OnStart callback for target: {}: {}",
                  name_, e.what());
        }
    }
}

void Target::notifyEnd(TargetStatus status) {
    TargetEndCallback callbackCopy;
    {
        std::shared_lock lock(callbackMutex_);
        callbackCopy = onEnd_;
    }
    if (callbackCopy) {
        try {
            callbackCopy(name_, status);
            LOG_F(INFO,
                  "OnEnd callback executed for target: {} with status: {}",
                  name_, static_cast<int>(status));
        } catch (const std::exception& e) {
            LOG_F(ERROR, "Exception in OnEnd callback for target: {}: {}",
                  name_, e.what());
        }
    }
}

void Target::notifyError(const std::exception& e) {
    TargetErrorCallback callbackCopy;
    {
        std::shared_lock lock(callbackMutex_);
        callbackCopy = onError_;
    }
    if (callbackCopy) {
        try {
            callbackCopy(name_, e);
            LOG_F(INFO,
                  "OnError callback executed for target: {} with error: {}",
                  name_, e.what());
        } catch (const std::exception& ex) {
            LOG_F(ERROR, "Exception in OnError callback for target: {}: {}",
                  name_, ex.what());
        }
    }
}

void Target::execute() {
    if (!isEnabled()) {
        status_ = TargetStatus::Skipped;
        LOG_F(WARNING, "Target {} is disabled, skipping execution", name_);
        notifyEnd(status_);
        return;
    }

    if (tasks_.empty()) {
        status_ = TargetStatus::Completed;
        LOG_F(WARNING, "Target {} has no tasks, skipping execution", name_);
        notifyEnd(status_);
        return;
    }

    if (!queue_ || queue_->empty()) {
        status_ = TargetStatus::Failed;
        LOG_F(ERROR, "Task queue is empty, cannot execute target {}", name_);
        notifyEnd(status_);
        return;
    }

    status_ = TargetStatus::InProgress;
    LOG_F(INFO, "Target {} execution started", name_);
    notifyStart();

    for (auto& task : tasks_) {
        if (status_ == TargetStatus::Failed ||
            status_ == TargetStatus::Skipped) {
            break;
        }

        int attempt = 0;
        bool success = false;

        while (attempt <= maxRetries_) {
            try {
                LOG_F(INFO, "Executing task {} for target {}, attempt {}",
                      task->getName(), name_, attempt + 1);
                // Get the params from the queue
                // Max: If the task has no params, it still needs en empty json
                auto paramsOpt = queue_->find(task->getUUID());
                if (!paramsOpt) {
                    THROW_TASK_ERROR_EXCEPTION(
                        "Task parameters not found in the queue");
                }
                // Execute the task
                task->execute(paramsOpt.value());
                if (task->getStatus() == TaskStatus::Failed) {
                    THROW_TASK_ERROR_EXCEPTION("Task execution failed");
                }
                success = true;
                break;
            } catch (const std::exception& e) {
                attempt++;
                LOG_F(
                    ERROR,
                    "Task {} execution failed for target {} on attempt {}: {}",
                    task->getName(), name_, attempt, e.what());
                if (attempt > maxRetries_) {
                    notifyError(e);
                    status_ = TargetStatus::Failed;
                    notifyEnd(status_);
                    return;
                }
            }
        }

        if (success) {
            completedTasks_.fetch_add(1);
            LOG_F(INFO, "Task {} completed successfully for target {}",
                  task->getName(), name_);
        }
    }

    if (status_ != TargetStatus::Failed) {
        status_ = TargetStatus::Completed;
        LOG_F(INFO, "Target {} execution completed successfully", name_);
        notifyEnd(status_);
        std::this_thread::sleep_for(cooldown_);
        LOG_F(INFO, "Target {} cooldown period of {}s completed", name_,
              cooldown_.count());
    }
}

void Target::loadTasksFromJson(const json& tasksJson) {
    for (const auto& taskJson : tasksJson) {
        std::string taskName = taskJson.at("name").get<std::string>();
        using namespace matchit;
        auto task = match(taskName)(
            pattern | "TakeExposure" = [&]() -> std::unique_ptr<Task> {
                auto task = TaskCreator<task::TakeExposureTask>::createTask();
                return task;
            },
            pattern | "TakeManyExposure" = [&]() -> std::unique_ptr<Task> {
                auto task =
                    TaskCreator<task::TakeManyExposureTask>::createTask();
                return task;
            },
            pattern | "SubframeExposure" = [&]() -> std::unique_ptr<Task> {
                auto task =
                    TaskCreator<task::SubframeExposureTask>::createTask();
                return task;
            },
            pattern | _ = [&]() -> std::unique_ptr<Task> {
                THROW_TASK_ERROR_EXCEPTION("Unknown task type: {}", taskName);
            });
        addTask(std::move(task));
    }
}

}  // namespace lithium::sequencer