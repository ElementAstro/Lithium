#include "sequencer.hpp"
#include "task.hpp"

#include "atom/log/loguru.hpp"

Target::Target(std::string name, int delayAfterTarget, int priority)
    : name_(std::move(name)),
      delayAfterTarget_(delayAfterTarget),
      priority_(priority),
      enabled_(true) {}

void Target::addTask(std::shared_ptr<Task> task) {
    tasks_.emplace_back(std::move(task));
}

void Target::setDelayAfterTarget(int delay) { delayAfterTarget_ = delay; }

void Target::setPriority(int p) { priority_ = p; }

auto Target::getPriority() const -> int { return priority_; }

void Target::enable() { enabled_ = true; }

void Target::disable() { enabled_ = false; }

auto Target::isEnabled() const -> bool { return enabled_; }

void Target::execute(std::stop_token stopToken, std::atomic<bool>& pauseFlag,
                     std::condition_variable_any& cv, std::mutex& mtx) {
    if (enabled_) {
        LOG_F(INFO, "Starting target: {}", name_);
        for (const auto& task : tasks_) {
            if (stopToken.stop_requested()) {
                return;
            }

            std::unique_lock lock(mtx);
            cv.wait(lock, [&] {
                return !pauseFlag.load() || stopToken.stop_requested();
            });

            if (stopToken.stop_requested()) {
                return;
            }

            try {
                task->start();
                task->run();
                if (task->isTimeout()) {
                    LOG_F(ERROR, "Task {} timed out.", task->getName());
                    task->fail(std::runtime_error("Timeout"));
                }
            } catch (const std::exception& ex) {
                LOG_F(ERROR, "Task {} failed with exception: {}",
                      task->getName(), ex.what());
            }
        }
        LOG_F(INFO, "Completed target: {}", name_);
        std::this_thread::sleep_for(std::chrono::seconds(delayAfterTarget_));
    } else {
        LOG_F(WARNING, "Target {} is disabled.", name_);
    }
}

auto Target::getName() const -> std::string { return name_; }

// ExposureSequence Class Implementation

ExposureSequence::ExposureSequence()
    : stopFlag_(false), pauseFlag_(false), sequenceThread_(nullptr) {}

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

void ExposureSequence::modifyTarget(size_t index, std::optional<int> newDelay,
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
    stopFlag_.store(false);
    pauseFlag_.store(false);
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
    pauseFlag_.store(true);
    LOG_F(INFO, "Pausing all tasks.");
}

void ExposureSequence::resume() {
    pauseFlag_.store(false);
    cv_.notify_all();
    LOG_F(INFO, "Resuming all tasks.");
}

void ExposureSequence::executeSequence(std::stop_token stopToken) {
    for (const auto& target : targets_) {
        if (stopToken.stop_requested()) {
            return;
        }
        if (target->isEnabled()) {
            target->execute(stopToken, pauseFlag_, cv_, mutex_);
        }
    }
}
