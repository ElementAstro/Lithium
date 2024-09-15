#ifndef LITHIUM_TARGET_HPP
#define LITHIUM_TARGET_HPP

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "task.hpp"

namespace lithium::sequencer {

enum class TargetStatus { Pending, InProgress, Completed, Failed, Skipped };

class Target {
public:
    Target(std::string name,
           std::chrono::seconds cooldown = std::chrono::seconds{0});

    void addTask(std::unique_ptr<Task> task);
    void setCooldown(std::chrono::seconds cooldown);
    void setEnabled(bool enabled);

    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] TargetStatus getStatus() const;
    [[nodiscard]] bool isEnabled() const;

    void execute();

private:
    std::string name_;
    std::vector<std::unique_ptr<Task>> tasks_;
    std::chrono::seconds cooldown_;
    bool enabled_{true};
    TargetStatus status_{TargetStatus::Pending};
};

using TargetModifier = std::function<void(Target&)>;

}  // namespace lithium::sequencer

#endif  // LITHIUM_TARGET_HPP
