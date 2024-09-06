#ifndef LITHIUM_TASK_SEQUENCER_HPP
#define LITHIUM_TASK_SEQUENCER_HPP

#include <atomic>
#include <chrono>
#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "./task.hpp"
#include "target.hpp"

namespace lithium::sequencer {
enum class SequenceState { Idle, Running, Paused, Stopped, Stopping };
class ExposureSequence {
public:
    ExposureSequence();
    ~ExposureSequence();

    void addTarget(std::unique_ptr<Target> target);
    void removeTarget(const std::string& name);
    void modifyTarget(const std::string& name, const TargetModifier& modifier);

    void executeAll();
    void stop();
    void pause();
    void resume();

    // New methods
    void saveSequence(const std::string& filename) const;
    void loadSequence(const std::string& filename);
    std::vector<std::string> getTargetNames() const;
    TargetStatus getTargetStatus(const std::string& name) const;

private:
    std::vector<std::unique_ptr<Target>> targets_;
    std::atomic<SequenceState> state_{SequenceState::Idle};
    std::jthread sequenceThread_;

    void executeSequence();
};

}  // namespace lithium::sequencer

#endif  // LITHIUM_TASK_SEQUENCER_HPP