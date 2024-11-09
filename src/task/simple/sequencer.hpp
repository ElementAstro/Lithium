#ifndef LITHIUM_TASK_SEQUENCER_HPP
#define LITHIUM_TASK_SEQUENCER_HPP

#include <atomic>
#include <functional>
#include <memory>
#include <shared_mutex>
#include <string>
#include <thread>
#include <vector>

#include "target.hpp"

namespace lithium::sequencer {

// 枚举表示序列的状态
enum class SequenceState { Idle, Running, Paused, Stopping, Stopped };

// 假设 TargetStatus 已在 target.hpp 中定义
// enum class TargetStatus { Pending, Running, Completed, Failed, Skipped };

class ExposureSequence {
public:
    // 回调函数类型定义
    using SequenceCallback = std::function<void()>;
    using TargetCallback =
        std::function<void(const std::string& targetName, TargetStatus status)>;
    using ErrorCallback = std::function<void(const std::string& targetName,
                                             const std::exception& e)>;

    ExposureSequence();
    ~ExposureSequence();

    // 禁止拷贝
    ExposureSequence(const ExposureSequence&) = delete;
    ExposureSequence& operator=(const ExposureSequence&) = delete;

    // 目标管理
    void addTarget(std::unique_ptr<Target> target);
    void removeTarget(const std::string& name);
    void modifyTarget(const std::string& name, const TargetModifier& modifier);

    // 执行控制
    void executeAll();
    void stop();
    void pause();
    void resume();

    // 序列化
    void saveSequence(const std::string& filename) const;
    void loadSequence(const std::string& filename);

    // 查询函数
    std::vector<std::string> getTargetNames() const;
    TargetStatus getTargetStatus(const std::string& name) const;
    double getProgress() const;  // 返回进度百分比

    // 回调设置函数
    void setOnSequenceStart(SequenceCallback callback);
    void setOnSequenceEnd(SequenceCallback callback);
    void setOnTargetStart(TargetCallback callback);
    void setOnTargetEnd(TargetCallback callback);
    void setOnError(ErrorCallback callback);

private:
    std::vector<std::unique_ptr<Target>> targets_;
    mutable std::shared_mutex mutex_;
    std::atomic<SequenceState> state_{SequenceState::Idle};
    std::jthread sequenceThread_;

    // 进度跟踪
    std::atomic<size_t> completedTargets_{0};
    size_t totalTargets_ = 0;

    // 回调函数
    SequenceCallback onSequenceStart_;
    SequenceCallback onSequenceEnd_;
    TargetCallback onTargetStart_;
    TargetCallback onTargetEnd_;
    ErrorCallback onError_;

    // 辅助方法
    void executeSequence();
    void notifySequenceStart();
    void notifySequenceEnd();
    void notifyTargetStart(const std::string& name);
    void notifyTargetEnd(const std::string& name, TargetStatus status);
    void notifyError(const std::string& name, const std::exception& e);
};

}  // namespace lithium::sequencer

#endif  // LITHIUM_TASK_SEQUENCER_HPP
