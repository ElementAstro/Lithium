#ifndef LITHIUM_TASK_HPP
#define LITHIUM_TASK_HPP

#include <chrono>
#include <functional>
#include <optional>
#include <string>

namespace lithium::sequencer {

enum class TaskStatus { Pending, InProgress, Completed, Failed };

class Task {
public:
    Task(std::string name, std::function<void()> action);

    void execute();
    void setTimeout(std::chrono::seconds timeout);

    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] TaskStatus getStatus() const;
    [[nodiscard]] std::optional<std::string> getError() const;

private:
    std::string name_;
    std::function<void()> action_;
    std::chrono::seconds timeout_{0};
    TaskStatus status_{TaskStatus::Pending};
    std::optional<std::string> error_;
};

}  // namespace lithium::sequencer

#endif  // LITHIUM_TASK_HPP
