#include "target.hpp"
#include <thread>

namespace lithium::sequencer {

Target::Target(std::string name, std::chrono::seconds cooldown)
    : name_(std::move(name)), cooldown_(cooldown) {}

void Target::addTask(std::unique_ptr<Task> task) {
    tasks_.push_back(std::move(task));
}

void Target::setCooldown(std::chrono::seconds cooldown) {
    cooldown_ = cooldown;
}

void Target::setEnabled(bool enabled) { enabled_ = enabled; }

const std::string& Target::getName() const { return name_; }

TargetStatus Target::getStatus() const { return status_; }

bool Target::isEnabled() const { return enabled_; }

void Target::execute() {
    if (!enabled_) {
        status_ = TargetStatus::Skipped;
        return;
    }

    status_ = TargetStatus::InProgress;
    for (auto& task : tasks_) {
        task->execute();
        if (task->getStatus() == TaskStatus::Failed) {
            status_ = TargetStatus::Failed;
            return;
        }
    }
    status_ = TargetStatus::Completed;

    std::this_thread::sleep_for(cooldown_);
}

}  // namespace lithium::sequencer