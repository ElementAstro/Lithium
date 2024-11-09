#include "target.hpp"

#include <mutex>
#include <stdexcept>
#include <thread>

namespace lithium::sequencer {

Target::Target(std::string name, std::chrono::seconds cooldown, int maxRetries)
    : name_(std::move(name)), cooldown_(cooldown), maxRetries_(maxRetries) {}

void Target::addTask(std::unique_ptr<Task> task) {
    if (!task) {
        throw std::invalid_argument("无法添加空任务");
    }
    std::unique_lock lock(mutex_);
    tasks_.emplace_back(std::move(task));
    totalTasks_ = tasks_.size();
}

void Target::setCooldown(std::chrono::seconds cooldown) {
    std::unique_lock lock(mutex_);
    cooldown_ = cooldown;
}

void Target::setEnabled(bool enabled) {
    std::unique_lock lock(mutex_);
    enabled_ = enabled;
}

void Target::setMaxRetries(int retries) {
    std::unique_lock lock(mutex_);
    maxRetries_ = retries;
}

void Target::setOnStart(TargetStartCallback callback) {
    std::unique_lock lock(callbackMutex_);
    onStart_ = std::move(callback);
}

void Target::setOnEnd(TargetEndCallback callback) {
    std::unique_lock lock(callbackMutex_);
    onEnd_ = std::move(callback);
}

void Target::setOnError(TargetErrorCallback callback) {
    std::unique_lock lock(callbackMutex_);
    onError_ = std::move(callback);
}

void Target::setStatus(TargetStatus status) {
    std::unique_lock lock(mutex_);
    status_ = status;
}

const std::string& Target::getName() const { return name_; }

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
        } catch (...) {
            // 记录回调异常，防止影响主流程
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
        } catch (...) {
            // 记录回调异常，防止影响主流程
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
        } catch (...) {
            // 记录回调异常，防止影响主流程
        }
    }
}

void Target::execute() {
    if (!isEnabled()) {
        status_ = TargetStatus::Skipped;
        notifyEnd(status_);
        return;
    }

    status_ = TargetStatus::InProgress;
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
                task->execute();
                if (task->getStatus() == TaskStatus::Failed) {
                    throw std::runtime_error("任务执行失败");
                }
                success = true;
                break;
            } catch (const std::exception& e) {
                attempt++;
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
        }
    }

    if (status_ != TargetStatus::Failed) {
        status_ = TargetStatus::Completed;
        notifyEnd(status_);
        std::this_thread::sleep_for(cooldown_);
    }
}

}  // namespace lithium::sequencer
