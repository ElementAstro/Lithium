#include "sequencer.hpp"

#include "atom/log/loguru.hpp"

Target::Target(std::string name, int delayAfterTarget, int priority)
    : name_(std::move(name)),
      delayAfterTarget_(delayAfterTarget),
      priority_(priority) {}

void Target::addTask(std::shared_ptr<Task> task) {
    tasks_.push_back(std::move(task));
}

void Target::setDelayAfterTarget(int delay) { delayAfterTarget_ = delay; }

void Target::setPriority(int p) { priority_ = p; }

auto Target::getPriority() const -> int { return priority_; }

void Target::enable() { enabled_ = true; }

void Target::disable() { enabled_ = false; }

auto Target::isEnabled() const -> bool { return enabled_; }

void Target::execute(std::atomic<bool>& stopFlag, std::atomic<bool>& pauseFlag,
                     std::condition_variable& cv, std::mutex& mtx) {
    if (enabled_) {
        LOG_F(INFO, "Starting target: {}", name_);
        for (const auto& task : tasks_) {
            if (stopFlag.load()) {
                return;
            }
            task->start();
            task->run();  // This will handle task execution and status updates
                          // internally
            if (task->isTimeout()) {
                LOG_F(ERROR, "Task {} timed out.", task->getName());
                task->fail(std::runtime_error("Timeout"));
            }
        }
        LOG_F(INFO, "Completed target: {}", name_);
        std::this_thread::sleep_for(std::chrono::seconds(delayAfterTarget_));
    } else {
        LOG_F(WARNING, "Target {} is disabled.", name_);
    }
}

auto Target::getName() const -> std::string { return name_; }

ExposureSequence::ExposureSequence()
    : stopFlag_(false), pauseFlag_(false), sequenceThread_(nullptr) {}

ExposureSequence::~ExposureSequence() {
    if (sequenceThread_ && sequenceThread_->joinable()) {
        stop();
        sequenceThread_->join();
    }
}

void ExposureSequence::addTarget(Target target) {
    std::unique_lock lock(mutex_);
    targets_.push_back(std::make_shared<Target>(std::move(target)));
    LOG_F(INFO, "Added target: {}", target.getName());
}

void ExposureSequence::removeTarget(size_t index) {
    std::unique_lock lock(mutex_);
    if (index < targets_.size()) {
        targets_.erase(targets_.begin() + index);
        LOG_F(INFO, "Removed target: {}", targets_[index]->getName());
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
    sequenceThread_ =
        std::make_unique<std::thread>(&ExposureSequence::executeSequence, this);
}

void ExposureSequence::stop() {
    stopFlag_.store(true);
    cv_.notify_all();
    LOG_F(INFO, "Stopping all tasks.");
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

void ExposureSequence::executeSequence() {
    for (const auto& target : targets_) {
        if (stopFlag_.load()) {
            return;
        }
        if (target->isEnabled()) {
            target->execute(stopFlag_, pauseFlag_, cv_, mutex_);
        }
    }
}
