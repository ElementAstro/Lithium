#include "task.hpp"

#include "atom/async/packaged_task.hpp"
#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"
#include "atom/utils/uuid.hpp"

namespace lithium::sequencer {
class TaskTimeoutException : public atom::error::RuntimeError {
public:
    using atom::error::RuntimeError::RuntimeError;
};

#define THROW_TASK_TIMEOUT_EXCEPTION(...)                                      \
    throw TaskTimeoutException(ATOM_FILE_NAME, ATOM_FILE_LINE, ATOM_FUNC_NAME, \
                               __VA_ARGS__);

Task::Task(std::string name, std::function<void(const json&)> action)
    : name_(std::move(name)),
      uuid_(atom::utils::UUID().toString()),
      action_(std::move(action)) {
    LOG_F(INFO, "Task created with name: {}, uuid: {}", name_, uuid_);
}

void Task::execute(const json& params) {
    LOG_F(INFO, "Task {} with uuid {} executing", name_, uuid_);
    status_ = TaskStatus::InProgress;
    error_.reset();

    try {
        if (timeout_ > std::chrono::seconds{0}) {
            LOG_F(INFO, "Task {} with uuid {} executing with timeout {}s",
                  name_, uuid_, timeout_.count());
            atom::async::EnhancedPackagedTask<void, const json&> task(action_);
            auto future = task.getEnhancedFuture();
            task(params);
            if (!future.waitFor(timeout_)) {
                THROW_TASK_TIMEOUT_EXCEPTION("Task timed out");
            }
        } else {
            LOG_F(INFO, "Task {} with uuid {} executing without timeout", name_,
                  uuid_);
            action_(params);
        }
        status_ = TaskStatus::Completed;
    } catch (const std::exception& e) {
        LOG_F(ERROR, "Task {} with uuid {} failed: {}", name_, uuid_, e.what());
        status_ = TaskStatus::Failed;
        error_ = e.what();
    }
    LOG_F(INFO, "Task {} with uuid {} completed", name_, uuid_);
}

void Task::setTimeout(std::chrono::seconds timeout) { timeout_ = timeout; }

auto Task::getName() const -> const std::string& { return name_; }

auto Task::getUUID() const -> const std::string& { return uuid_; }

auto Task::getStatus() const -> TaskStatus { return status_; }

auto Task::getError() const -> std::optional<std::string> { return error_; }

}  // namespace lithium::sequencer