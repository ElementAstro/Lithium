// target.hpp
#ifndef LITHIUM_TARGET_HPP
#define LITHIUM_TARGET_HPP

#include <atomic>
#include <chrono>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <vector>

#include "task.hpp"

namespace lithium::sequencer {
// 目标状态枚举
enum class TargetStatus { Pending, InProgress, Completed, Failed, Skipped };

// 回调函数类型定义
using TargetStartCallback = std::function<void(const std::string&)>;
using TargetEndCallback = std::function<void(const std::string&, TargetStatus)>;
using TargetErrorCallback =
    std::function<void(const std::string&, const std::exception&)>;

class Target;
// 目标修改器类型定义
using TargetModifier = std::function<void(Target&)>;

class Target {
public:
    Target(std::string name,
           std::chrono::seconds cooldown = std::chrono::seconds{0},
           int maxRetries = 0);

    // 禁止拷贝
    Target(const Target&) = delete;
    Target& operator=(const Target&) = delete;

    // 目标管理
    void addTask(std::unique_ptr<Task> task);
    void setCooldown(std::chrono::seconds cooldown);
    void setEnabled(bool enabled);
    void setMaxRetries(int retries);
    void setStatus(TargetStatus status);

    // 回调设置
    void setOnStart(TargetStartCallback callback);
    void setOnEnd(TargetEndCallback callback);
    void setOnError(TargetErrorCallback callback);

    // 查询函数
    [[nodiscard]] const std::string& getName() const;
    [[nodiscard]] TargetStatus getStatus() const;
    [[nodiscard]] bool isEnabled() const;
    [[nodiscard]] double getProgress() const;  // 返回进度百分比

    // 执行函数
    void execute();

private:
    std::string name_;
    std::vector<std::unique_ptr<Task>> tasks_;
    std::chrono::seconds cooldown_;
    bool enabled_{true};
    std::atomic<TargetStatus> status_{TargetStatus::Pending};
    std::shared_mutex mutex_;

    // 进度跟踪
    std::atomic<size_t> completedTasks_{0};
    size_t totalTasks_ = 0;

    // 回调函数
    TargetStartCallback onStart_;
    TargetEndCallback onEnd_;
    TargetErrorCallback onError_;

    // 重试机制
    int maxRetries_;
    mutable std::shared_mutex callbackMutex_;

    // 辅助方法
    void notifyStart();
    void notifyEnd(TargetStatus status);
    void notifyError(const std::exception& e);
};

}  // namespace lithium::sequencer

#endif  // LITHIUM_TARGET_HPP
