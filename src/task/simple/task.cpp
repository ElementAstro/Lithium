#include "task.hpp"
#include <future>

namespace lithium::sequencer {

Task::Task(std::string name, std::function<void()> action)
    : name_(std::move(name)), action_(std::move(action)) {}

void Task::execute() {
    status_ = TaskStatus::InProgress;
    error_.reset();

    try {
        if (timeout_ > std::chrono::seconds{0}) {
            auto future = std::async(std::launch::async, action_);
            if (future.wait_for(timeout_) == std::future_status::timeout) {
                throw std::runtime_error("Task timed out");
            }
        } else {
            action_();
        }
        status_ = TaskStatus::Completed;
    } catch (const std::exception& e) {
        status_ = TaskStatus::Failed;
        error_ = e.what();
    }
}

void Task::setTimeout(std::chrono::seconds timeout) { timeout_ = timeout; }

const std::string& Task::getName() const { return name_; }

TaskStatus Task::getStatus() const { return status_; }

std::optional<std::string> Task::getError() const { return error_; }

}  // namespace lithium::sequencer
