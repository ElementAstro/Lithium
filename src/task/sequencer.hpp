#ifndef LITHIUM_TASK_SEQUENCER_HPP
#define LITHIUM_TASK_SEQUENCER_HPP

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <ostream>
#include <queue>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <atomic>

namespace lithium::task {
    
enum class ExposureType { Light, Dark, Flat, Bias };

struct ExposureParams {
    int exposureTime;
    int gain;
    int offset;
    int binning;
} __attribute__((aligned(16)));

class ExposureTask {
public:
    ExposureTask(std::string name, ExposureType type, ExposureParams params, int delayAfterTask = 0)
        : name_(std::move(name)), type_(type), params_(std::move(params)), , delayAfterTask_(delayAfterTask) {}

    void setParams(ExposureParams newParams) { params_ = std::move(newParams); }

    void setDelayAfterTask(int delay) { delayAfterTask_ = delay; }

    void enable() { enabled_ = true; }

    void disable() { enabled_ = false; }

    bool isEnabled() const { return enabled_; }

    void execute(std::atomic<bool>& stopFlag, std::atomic<bool>& pauseFlag, std::condition_variable& cv, std::mutex& mtx) {
        if (enabled_) {
            std::cout << "Executing " << name_ << " task with type " << exposureTypeToString(type_) << " and parameters: "
                      << "ExposureTime=" << params_.exposureTime << ", "
                      << "Gain=" << params_.gain << ", "
                      << "Offset=" << params_.offset << ", "
                      << "Binning=" << params_.binning << std::endl;

            auto start = std::chrono::high_resolution_clock::now();
            
            for (int i = 0; i < params_.exposureTime; ++i) {
                if (stopFlag.load()) {
                    return;
                }
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [&pauseFlag]() { return !pauseFlag.load(); });
            }

            std::this_thread::sleep_for(std::chrono::seconds(delayAfterTask_));
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
            std::cout << "Task " << name_ << " completed in " << duration << " seconds." << std::endl;
        } else {
            std::cout << name_ << " task is disabled." << std::endl;
        }
    }

private:
    std::string name_;
    ExposureType type_;
    ExposureParams params_;
    bool enabled_{};
    int delayAfterTask_;

    std::string exposureTypeToString(ExposureType type) const {
        switch (type) {
            case ExposureType::Light: return "Light";
            case ExposureType::Dark: return "Dark";
            case ExposureType::Flat: return "Flat";
            case ExposureType::Bias: return "Bias";
        }
        return "Unknown";
    }
};

class Target {
public:
    Target(std::string name, int delayAfterTarget = 0, int priority = 0)
        : name_(std::move(name)), delayAfterTarget_(delayAfterTarget), priority_(priority) {}

    void addTask(ExposureTask task) {
        tasks_.push_back(std::make_shared<ExposureTask>(std::move(task)));
    }

    void setDelayAfterTarget(int delay) { delayAfterTarget_ = delay; }

    void setPriority(int p) { priority_ = p; }

    int getPriority() const { return priority_; }

    void enable() { enabled_ = true; }

    void disable() { enabled_ = false; }

    bool isEnabled() const { return enabled_; }

    void execute(std::atomic<bool>& stopFlag, std::atomic<bool>& pauseFlag, std::condition_variable& cv, std::mutex& mtx) {
        if (enabled_) {
            std::cout << "Starting target: " << name_ << std::endl;
            for (const auto& task : tasks_) {
                if (stopFlag.load()) {
                    return;
                }
                task->execute(stopFlag, pauseFlag, cv, mtx);
            }
            std::cout << "Completed target: " << name_ << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(delayAfterTarget_));
        } else {
            std::cout << name_ << " target is disabled." << std::endl;
        }
    }

    std::string getName() const { return name_; }

private:
    std::string name_;
    std::vector<std::shared_ptr<ExposureTask>> tasks_;
    int delayAfterTarget_;
    int priority_;
    bool enabled_ = true;
};

class ExposureSequence {
public:
    ExposureSequence() : stopFlag_(false), pauseFlag_(false), sequenceThread_(nullptr) {}

    ~ExposureSequence() {
        if (sequenceThread_ && sequenceThread_->joinable()) {
            stop();
            sequenceThread_->join();
        }
    }

    void addTarget(Target target) {
        std::unique_lock lock(mutex_);
        targets_.push_back(std::make_shared<Target>(std::move(target)));
        log("Added target: " + target.getName());
    }

    void removeTarget(size_t index) {
        std::unique_lock lock(mutex_);
        if (index < targets_.size()) {
            targets_.erase(targets_.begin() + index);
            log("Removed target at index " + std::to_string(index));
        } else {
            log("Error: Target index out of range for removal.");
        }
    }

    void modifyTarget(size_t index, std::optional<int> newDelay = std::nullopt, std::optional<int> newPriority = std::nullopt) {
        std::unique_lock lock(mutex_);
        if (index < targets_.size()) {
            if (newDelay) {
                targets_[index]->setDelayAfterTarget(*newDelay);
            }
            if (newPriority) {
                targets_[index]->setPriority(*newPriority);
            }
            log("Modified target at index " + std::to_string(index));
        } else {
            log("Error: Target index out of range for modification.");
        }
    }

    void enableTarget(size_t index) {
        std::unique_lock lock(mutex_);
        if (index < targets_.size()) {
            targets_[index]->enable();
            log("Enabled target at index " + std::to_string(index));
        } else {
            log("Error: Target index out of range for enabling.");
        }
    }

    void disableTarget(size_t index) {
        std::unique_lock lock(mutex_);
        if (index < targets_.size()) {
            targets_[index]->disable();
            log("Disabled target at index " + std::to_string(index));
        } else {
            log("Error: Target index out of range for disabling.");
        }
    }

    void executeAll() {
        stopFlag_.store(false);
        pauseFlag_.store(false);
        if (sequenceThread_ && sequenceThread_->joinable()) {
            sequenceThread_->join();
        }
        sequenceThread_ = std::make_unique<std::thread>(&ExposureSequence::executeSequence, this);
    }

    void stop() {
        stopFlag_.store(true);
        cv_.notify_all();
        log("Stopped all tasks.");
    }

    void pause() {
        pauseFlag_.store(true);
        log("Paused all tasks.");
    }

    void resume() {
        pauseFlag_.store(false);
        cv_.notify_all();
        log("Resumed all tasks.");
    }

private:
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::shared_ptr<Target>> targets_;
    std::atomic<bool> stopFlag_;
    std::atomic<bool> pauseFlag_;
    std::unique_ptr<std::thread> sequenceThread_;

    void log(const std::string& message) const {
       // std::unique_lock lock(mutex_);
        std::cout << "[LOG]: " << message << std::endl;
    }

    void executeSequence() {
        for (const auto& target : targets_) {
            if (stopFlag_.load()) {
                return;
            }
            if (target->isEnabled()) {
                target->execute(stopFlag_, pauseFlag_, cv_, mutex_);
            }
        }
    }
};
}


#endif