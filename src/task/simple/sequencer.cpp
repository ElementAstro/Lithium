#include "sequencer.hpp"
#include <algorithm>
#include <fstream>

#include "atom/type/json.hpp"

namespace lithium::sequencer {

ExposureSequence::ExposureSequence() = default;

ExposureSequence::~ExposureSequence() { stop(); }

void ExposureSequence::addTarget(std::unique_ptr<Target> target) {
    targets_.push_back(std::move(target));
}

void ExposureSequence::removeTarget(const std::string& name) {
    targets_.erase(std::remove_if(targets_.begin(), targets_.end(),
                                  [&name](const auto& target) {
                                      return target->getName() == name;
                                  }),
                   targets_.end());
}

void ExposureSequence::modifyTarget(const std::string& name,
                                    const TargetModifier& modifier) {
    auto it = std::find_if(
        targets_.begin(), targets_.end(),
        [&name](const auto& target) { return target->getName() == name; });
    if (it != targets_.end()) {
        modifier(**it);
    }
}

void ExposureSequence::executeAll() {
    if (state_.exchange(SequenceState::Running) != SequenceState::Idle) {
        return;
    }
    sequenceThread_ = std::jthread([this] { executeSequence(); });
}

void ExposureSequence::stop() {
    state_.store(SequenceState::Stopping);
    if (sequenceThread_.joinable()) {
        sequenceThread_.request_stop();
        sequenceThread_.join();
    }
    state_.store(SequenceState::Idle);
}

void ExposureSequence::pause() {
    SequenceState expected = SequenceState::Running;
    state_.compare_exchange_strong(expected, SequenceState::Paused);
}

void ExposureSequence::resume() {
    SequenceState expected = SequenceState::Paused;
    state_.compare_exchange_strong(expected, SequenceState::Running);
}

void ExposureSequence::saveSequence(const std::string& filename) const {
    nlohmann::json j;
    for (const auto& target : targets_) {
        j["targets"].push_back({
            {"name", target->getName()}, {"enabled", target->isEnabled()},
            // Add more target properties as needed
        });
    }
    std::ofstream file(filename);
    file << j.dump(4);
}

void ExposureSequence::loadSequence(const std::string& filename) {
    std::ifstream file(filename);
    nlohmann::json j;
    file >> j;

    targets_.clear();
    for (const auto& targetJson : j["targets"]) {
        auto target = std::make_unique<Target>(targetJson["name"]);
        target->setEnabled(targetJson["enabled"]);
        // Load more target properties as needed
        targets_.push_back(std::move(target));
    }
}

std::vector<std::string> ExposureSequence::getTargetNames() const {
    std::vector<std::string> names;
    std::transform(targets_.begin(), targets_.end(), std::back_inserter(names),
                   [](const auto& target) { return target->getName(); });
    return names;
}

TargetStatus ExposureSequence::getTargetStatus(const std::string& name) const {
    auto it = std::find_if(
        targets_.begin(), targets_.end(),
        [&name](const auto& target) { return target->getName() == name; });
    return it != targets_.end() ? (*it)->getStatus() : TargetStatus::Skipped;
}

void ExposureSequence::executeSequence() {
    for (auto& target : targets_) {
        if (state_.load() == SequenceState::Stopping) {
            break;
        }
        while (state_.load() == SequenceState::Paused) {
            std::this_thread::yield();
        }
        if (target->isEnabled()) {
            target->execute();
        }
    }
    state_.store(SequenceState::Idle);
}

}  // namespace lithium::sequencer
