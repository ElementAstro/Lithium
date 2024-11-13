/**
 * @file sequencer.cpp
 * @brief Task Sequencer Implementation
 *
 * This file contains the implementation of the ExposureSequence class,
 * which manages and executes a sequence of targets.
 *
 * @date 2023-07-21
 * @modified 2024-04-27
 * @autor Max Qian <lightapt.com>
 * @copyright
 */

#include "sequencer.hpp"
#include <algorithm>
#include <chrono>
#include <fstream>
#include <mutex>
#include <stdexcept>

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace lithium::sequencer {

using json = nlohmann::json;

// ExposureSequence Implementation

ExposureSequence::ExposureSequence() = default;

ExposureSequence::~ExposureSequence() { stop(); }

void ExposureSequence::addTarget(std::unique_ptr<Target> target) {
    if (!target) {
        THROW_INVALID_ARGUMENT("Cannot add a null target");
    }
    std::unique_lock lock(mutex_);
    auto it = std::find_if(targets_.begin(), targets_.end(),
                           [&](const std::unique_ptr<Target>& t) {
                               return t->getUUID() == target->getUUID();
                           });
    if (it != targets_.end()) {
        THROW_RUNTIME_ERROR("Target with name '" + target->getUUID() +
                            "' already exists");
    }
    targets_.push_back(std::move(target));
    totalTargets_ = targets_.size();
}

void ExposureSequence::removeTarget(const std::string& name) {
    std::unique_lock lock(mutex_);
    auto it = std::remove_if(
        targets_.begin(), targets_.end(),
        [&name](const auto& target) { return target->getName() == name; });
    if (it == targets_.end()) {
        THROW_RUNTIME_ERROR("Target with name '" + name + "' not found");
    }
    targets_.erase(it, targets_.end());
    totalTargets_ = targets_.size();
}

void ExposureSequence::modifyTarget(const std::string& name,
                                    const TargetModifier& modifier) {
    std::shared_lock lock(mutex_);
    auto it = std::find_if(
        targets_.begin(), targets_.end(),
        [&name](const auto& target) { return target->getName() == name; });
    if (it != targets_.end()) {
        try {
            modifier(**it);
        } catch (const std::exception& e) {
            THROW_RUNTIME_ERROR("Failed to modify target '" + name +
                                "': " + e.what());
        }
    } else {
        THROW_RUNTIME_ERROR("Target with name '" + name + "' not found");
    }
}

void ExposureSequence::executeAll() {
    SequenceState expected = SequenceState::Idle;
    if (!state_.compare_exchange_strong(expected, SequenceState::Running)) {
        // 如果当前状态不是Idle，无法开始执行
        THROW_RUNTIME_ERROR("Sequence is not in Idle state");
    }

    completedTargets_.store(0);
    notifySequenceStart();

    // 在单独的线程中启动序列执行
    sequenceThread_ = std::jthread([this] { executeSequence(); });
}

void ExposureSequence::stop() {
    SequenceState current = state_.load();
    if (current == SequenceState::Idle || current == SequenceState::Stopped) {
        return;
    }

    state_.store(SequenceState::Stopping);
    if (sequenceThread_.joinable()) {
        sequenceThread_.request_stop();
        sequenceThread_.join();
    }
    state_.store(SequenceState::Idle);
    notifySequenceEnd();
}

void ExposureSequence::pause() {
    SequenceState expected = SequenceState::Running;
    if (!state_.compare_exchange_strong(expected, SequenceState::Paused)) {
        THROW_RUNTIME_ERROR("Cannot pause sequence. Current state: " +
                            std::to_string(static_cast<int>(state_.load())));
    }
}

void ExposureSequence::resume() {
    SequenceState expected = SequenceState::Paused;
    if (!state_.compare_exchange_strong(expected, SequenceState::Running)) {
        THROW_RUNTIME_ERROR("Cannot resume sequence. Current state: " +
                            std::to_string(static_cast<int>(state_.load())));
    }
}

void ExposureSequence::saveSequence(const std::string& filename) const {
    json j;
    std::shared_lock lock(mutex_);
    for (const auto& target : targets_) {
        json targetJson = {{"name", target->getName()},
                           {"enabled", target->isEnabled()},
                           {"tasks", json::array()}};
        /*
        for (const auto& task : target->getTasks()) {
            targetJson["tasks"].push_back({
                {"name", task->getName()},
                // Add more task properties as needed
            });
        }
        */
        j["targets"].push_back(targetJson);
    }
    std::ofstream file(filename);
    if (!file.is_open()) {
        THROW_RUNTIME_ERROR("Failed to open file '" + filename +
                            "' for writing");
    }
    file << j.dump(4);
}

void ExposureSequence::loadSequence(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        THROW_RUNTIME_ERROR("Failed to open file '" + filename +
                            "' for reading");
    }

    json j;
    file >> j;

    std::unique_lock lock(mutex_);
    targets_.clear();
    if (!j.contains("targets") || !j["targets"].is_array()) {
        THROW_RUNTIME_ERROR(
            "Invalid sequence file format: 'targets' array missing");
    }

    for (const auto& targetJson : j["targets"]) {
        if (!targetJson.contains("name") || !targetJson.contains("enabled")) {
            THROW_RUNTIME_ERROR("Invalid target format in sequence file");
        }
        std::string name = targetJson["name"].get<std::string>();
        bool enabled = targetJson["enabled"].get<bool>();
        auto target = std::make_unique<Target>(name);
        target->setEnabled(enabled);
        if (targetJson.contains("tasks") && targetJson["tasks"].is_array()) {
            target->loadTasksFromJson(targetJson["tasks"]);
        }
        targets_.push_back(std::move(target));
    }
    totalTargets_ = targets_.size();
}

std::vector<std::string> ExposureSequence::getTargetNames() const {
    std::shared_lock lock(mutex_);
    std::vector<std::string> names;
    names.reserve(targets_.size());
    for (const auto& target : targets_) {
        names.emplace_back(target->getName());
    }
    return names;
}

TargetStatus ExposureSequence::getTargetStatus(const std::string& name) const {
    std::shared_lock lock(mutex_);
    auto it = std::find_if(
        targets_.begin(), targets_.end(),
        [&name](const auto& target) { return target->getName() == name; });
    if (it != targets_.end()) {
        return (*it)->getStatus();
    }
    return TargetStatus::Skipped;  // 或其他默认值
}

double ExposureSequence::getProgress() const {
    size_t completed = completedTargets_.load();
    size_t total = totalTargets_;
    if (total == 0) {
        return 100.0;
    }
    return (static_cast<double>(completed) / static_cast<double>(total)) *
           100.0;
}

// 回调设置函数

void ExposureSequence::setOnSequenceStart(SequenceCallback callback) {
    std::unique_lock lock(mutex_);
    onSequenceStart_ = std::move(callback);
}

void ExposureSequence::setOnSequenceEnd(SequenceCallback callback) {
    std::unique_lock lock(mutex_);
    onSequenceEnd_ = std::move(callback);
}

void ExposureSequence::setOnTargetStart(TargetCallback callback) {
    std::unique_lock lock(mutex_);
    onTargetStart_ = std::move(callback);
}

void ExposureSequence::setOnTargetEnd(TargetCallback callback) {
    std::unique_lock lock(mutex_);
    onTargetEnd_ = std::move(callback);
}

void ExposureSequence::setOnError(ErrorCallback callback) {
    std::unique_lock lock(mutex_);
    onError_ = std::move(callback);
}

// 回调通知辅助方法

void ExposureSequence::notifySequenceStart() {
    SequenceCallback callbackCopy;
    {
        std::shared_lock lock(mutex_);
        callbackCopy = onSequenceStart_;
    }
    if (callbackCopy) {
        try {
            callbackCopy();
        } catch (...) {
            // 处理或记录回调异常
        }
    }
}

void ExposureSequence::notifySequenceEnd() {
    SequenceCallback callbackCopy;
    {
        std::shared_lock lock(mutex_);
        callbackCopy = onSequenceEnd_;
    }
    if (callbackCopy) {
        try {
            callbackCopy();
        } catch (...) {
            // 处理或记录回调异常
        }
    }
}

void ExposureSequence::notifyTargetStart(const std::string& name) {
    TargetCallback callbackCopy;
    {
        std::shared_lock lock(mutex_);
        callbackCopy = onTargetStart_;
    }
    if (callbackCopy) {
        try {
            callbackCopy(name, TargetStatus::InProgress);
        } catch (...) {
            // 处理或记录回调异常
        }
    }
}

void ExposureSequence::notifyTargetEnd(const std::string& name,
                                       TargetStatus status) {
    TargetCallback callbackCopy;
    {
        std::shared_lock lock(mutex_);
        callbackCopy = onTargetEnd_;
    }
    if (callbackCopy) {
        try {
            callbackCopy(name, status);
        } catch (...) {
            // 处理或记录回调异常
        }
    }
}

void ExposureSequence::notifyError(const std::string& name,
                                   const std::exception& e) {
    ErrorCallback callbackCopy;
    {
        std::shared_lock lock(mutex_);
        callbackCopy = onError_;
    }
    if (callbackCopy) {
        try {
            callbackCopy(name, e);
        } catch (...) {
            // 处理或记录回调异常
        }
    }
}

void ExposureSequence::executeSequence() {
    try {
        for (auto& target : targets_) {
            if (state_.load() == SequenceState::Stopping) {
                break;
            }

            // 处理暂停
            while (state_.load() == SequenceState::Paused) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                if (state_.load() == SequenceState::Stopping) {
                    break;
                }
            }

            if (state_.load() == SequenceState::Stopping) {
                break;
            }

            if (target->isEnabled()) {
                notifyTargetStart(target->getName());
                try {
                    target->execute();
                    target->setStatus(TargetStatus::Completed);
                    notifyTargetEnd(target->getName(), TargetStatus::Completed);
                } catch (const std::exception& e) {
                    target->setStatus(TargetStatus::Failed);
                    notifyTargetEnd(target->getName(), TargetStatus::Failed);
                    notifyError(target->getName(), e);
                }
                completedTargets_.fetch_add(1);
            } else {
                target->setStatus(TargetStatus::Skipped);
                notifyTargetEnd(target->getName(), TargetStatus::Skipped);
                completedTargets_.fetch_add(1);
            }

            // 检查是否有停止信号
            if (sequenceThread_.get_stop_token().stop_requested()) {
                state_.store(SequenceState::Stopping);
                break;
            }
        }
    } catch (const std::exception& e) {
        // 记录未捕获的异常，防止线程崩溃
        LOG_F(ERROR, "Unhandled exception in executeSequence: {}", e.what());
        // 可选：通过通用错误回调通知
    }

    // 完成序列状态
    state_.store(SequenceState::Idle);
    notifySequenceEnd();
}

}  // namespace lithium::sequencer
