#include "history.hpp"

#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>

#include "atom/type/json.hpp"
using json = nlohmann::json;

namespace lithium::debug {
CommandHistory::CommandHistory(size_t maxSize, std::string userName)
    : maxSize_(maxSize), userName_(std::move(userName)) {
    loadFromFile();
}

CommandHistory::~CommandHistory() { saveToFile(); }

void CommandHistory::addCommand(std::string command) {
    if (history_.size() >= maxSize_) {
        history_.pop_front();
    }
    history_.push_back({std::move(command), std::time(nullptr)});
    redoStack_.clear();
    updateFrequency(history_.back().command);
    saveToFile();
}

void CommandHistory::undo() {
    if (history_.empty()) {
        std::cout << "No commands to undo." << std::endl;
        return;
    }
    redoStack_.push_back(history_.back());
    history_.pop_back();
    saveToFile();
}

void CommandHistory::redo() {
    if (redoStack_.empty()) {
        std::cout << "No commands to redo." << std::endl;
        return;
    }
    history_.push_back(redoStack_.back());
    redoStack_.pop_back();
    saveToFile();
}

void CommandHistory::printHistory() const {
    for (const auto& entry : history_) {
        std::cout << std::format("{} {}", entry.command,
                                 std::asctime(std::localtime(&entry.timestamp)));
    }
}

void CommandHistory::search(const std::string& keyword) const {
    for (const auto& entry :
         history_ | std::views::filter([&](const auto& entry) {
             return entry.command.find(keyword) != std::string::npos;
         })) {
        std::cout << entry.command << " (Time: "
                  << std::asctime(std::localtime(&entry.timestamp)) << ")"
                  << std::endl;
    }
}

void CommandHistory::addAlias(const std::string& alias,
                              const std::string& command) {
    aliases_[alias] = command;
    saveToFile();
}

void CommandHistory::executeAlias(const std::string& alias) {
    if (auto it = aliases_.find(alias); it != aliases_.end()) {
        addCommand(it->second);
    } else {
        std::cout << "Alias not found." << std::endl;
    }
}

void CommandHistory::deleteCommand(size_t index) {
    if (index >= history_.size()) {
        std::cout << "Index out of range." << std::endl;
        return;
    }
    history_.erase(history_.begin() + index);
    saveToFile();
}

void CommandHistory::sortHistoryByTime() {
    std::ranges::sort(history_, {}, &CommandEntry::timestamp);
    saveToFile();
}

void CommandHistory::printFrequencyReport() const {
    std::cout << "Command Frequency Report:" << std::endl;
    for (const auto& [command, count] : frequency_) {
        std::cout << command << ": " << count << std::endl;
    }
}

void CommandHistory::filterHistoryByTime(const std::time_t startTime,
                                         const std::time_t endTime) const {
    for (const auto& entry :
         history_ | std::views::filter([&](const auto& entry) {
             return entry.timestamp >= startTime && entry.timestamp <= endTime;
         })) {
        std::cout << std::format("{} ({})", entry.timestamp,
                                 std::asctime(std::localtime(&entry.timestamp)))
                  << std::endl;
    }
}

void CommandHistory::clearHistory() {
    history_.clear();
    redoStack_.clear();
    frequency_.clear();
}

void CommandHistory::updateFrequency(const std::string& command) {
    frequency_[command]++;
}

void CommandHistory::saveToFile() const {
    json j;
    for (const auto& entry : history_) {
        j["history"].push_back(
            {{"command", entry.command}, {"timestamp", entry.timestamp}});
    }
    j["aliases"] = aliases_;
    j["frequency"] = frequency_;
    std::ofstream file(userName_ + "_history.json");
    file << j.dump(4);
}

void CommandHistory::loadFromFile() {
    std::ifstream file(userName_ + "_history.json");
    if (!file.is_open()) {
        return;
    }

    json j;
    file >> j;
    for (const auto& entry : j["history"]) {
        history_.push_back({entry["command"], entry["timestamp"]});
    }
    aliases_ = j["aliases"].get<decltype(aliases_)>();
    frequency_ = j["frequency"].get<decltype(frequency_)>();
}

}  // namespace lithium::debug
