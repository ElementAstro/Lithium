#include "history.hpp"

#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <mutex>
#include <ranges>

#include "atom/macro.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

using json = nlohmann::json;

namespace lithium::debug {

namespace {
std::mutex g_timeMutex;
std::string safeAsctime(const std::time_t* time) {
    std::lock_guard<std::mutex> lock(g_timeMutex);
    std::tm tm;
    localtime_r(time, &tm);
    char buffer[26];
    asctime_r(&tm, buffer);
    return std::string(buffer);
}
}  // namespace

struct CommandEntry {
    std::string command;    ///< The command string.
    std::time_t timestamp;  ///< The time when the command was added.
} ATOM_ALIGNAS(64);

struct CommandHistory::Impl {
    size_t maxSize;
    std::string userName;
    std::deque<CommandEntry> history;
    std::deque<CommandEntry> redoStack;
    std::unordered_map<std::string, std::string> aliases;
    std::unordered_map<std::string, int> frequency;

    Impl(size_t maxSize, std::string userName)
        : maxSize(maxSize), userName(std::move(userName)) {
        loadFromFile();
    }

    ~Impl() { saveToFile(); }

    void addCommand(std::string command) {
        if (history.size() >= maxSize) {
            history.pop_front();
        }
        history.push_back({std::move(command), std::time(nullptr)});
        redoStack.clear();
        updateFrequency(history.back().command);
        saveToFile();
    }

    void undo() {
        if (history.empty()) {
            std::cout << "No commands to undo." << std::endl;
            return;
        }
        redoStack.push_back(history.back());
        history.pop_back();
        saveToFile();
    }

    void redo() {
        if (redoStack.empty()) {
            std::cout << "No commands to redo." << std::endl;
            return;
        }
        history.push_back(redoStack.back());
        redoStack.pop_back();
        saveToFile();
    }

    void printHistory() const {
        for (const auto& entry : history) {
            std::cout << entry.command << " " << safeAsctime(&entry.timestamp);
        }
    }

    void search(const std::string& keyword) const {
        for (const auto& entry : history) {
            if (entry.command.find(keyword) != std::string::npos) {
                std::cout << entry.command
                          << " (Time: " << safeAsctime(&entry.timestamp) << ")"
                          << std::endl;
            }
        }
    }

    void addAlias(const std::string& alias, const std::string& command) {
        aliases[alias] = command;
        saveToFile();
    }

    void executeAlias(const std::string& alias) {
        if (auto aliasIter = aliases.find(alias); aliasIter != aliases.end()) {
            addCommand(aliasIter->second);
        } else {
            std::cout << "Alias not found." << std::endl;
        }
    }

    void deleteCommand(size_t index) {
        if (index >= history.size()) {
            std::cout << "Index out of range." << std::endl;
            return;
        }
        history.erase(history.begin() + static_cast<std::ptrdiff_t>(index));
        saveToFile();
    }

    void deleteCommands(const std::vector<size_t>& indices) {
        for (auto it = indices.rbegin(); it != indices.rend(); ++it) {
            if (*it < history.size()) {
                history.erase(history.begin() +
                              static_cast<std::ptrdiff_t>(*it));
            }
        }
        saveToFile();
    }

    void sortHistoryByTime() {
        std::ranges::sort(history, {}, &CommandEntry::timestamp);
        saveToFile();
    }

    void printFrequencyReport() const {
        std::cout << "Command Frequency Report:" << std::endl;
        for (const auto& [command, count] : frequency) {
            std::cout << command << ": " << count << std::endl;
        }
    }

    void filterHistoryByTime(const std::time_t startTime,
                             const std::time_t endTime) const {
        for (const auto& entry : history) {
            if (entry.timestamp >= startTime && entry.timestamp <= endTime) {
                std::cout << entry.command
                          << " (Time: " << safeAsctime(&entry.timestamp) << ")"
                          << std::endl;
            }
        }
    }

    void clearHistory() {
        history.clear();
        redoStack.clear();
        frequency.clear();
    }

    void exportHistory(const std::string& filePath) const {
        json jsonObj;
        for (const auto& entry : history) {
            jsonObj["history"].push_back(
                {{"command", entry.command}, {"timestamp", entry.timestamp}});
        }
        std::ofstream file(filePath);
        file << jsonObj.dump(4);
    }

    int getCommandFrequency(const std::string& command) const {
        if (auto it = frequency.find(command); it != frequency.end()) {
            return it->second;
        }
        return 0;
    }

private:
    void updateFrequency(const std::string& command) { frequency[command]++; }

    void saveToFile() const {
        json jsonObj;
        for (const auto& entry : history) {
            jsonObj["history"].push_back(
                {{"command", entry.command}, {"timestamp", entry.timestamp}});
        }
        jsonObj["aliases"] = aliases;
        jsonObj["frequency"] = frequency;
        std::ofstream file(userName + "_history.json");
        file << jsonObj.dump(4);
    }

    void loadFromFile() {
        std::ifstream file(userName + "_history.json");
        if (!file.is_open()) {
            return;
        }

        json jsonObj;
        file >> jsonObj;
        for (const auto& entry : jsonObj["history"]) {
            history.push_back({entry["command"], entry["timestamp"]});
        }
        aliases = jsonObj["aliases"].get<decltype(aliases)>();
        frequency = jsonObj["frequency"].get<decltype(frequency)>();
    }
};

CommandHistory::CommandHistory(size_t maxSize, std::string userName)
    : impl_(std::make_unique<Impl>(maxSize, std::move(userName))) {}

CommandHistory::~CommandHistory() = default;

void CommandHistory::addCommand(std::string command) {
    impl_->addCommand(std::move(command));
}

void CommandHistory::undo() { impl_->undo(); }

void CommandHistory::redo() { impl_->redo(); }

void CommandHistory::printHistory() const { impl_->printHistory(); }

void CommandHistory::search(const std::string& keyword) const {
    impl_->search(keyword);
}

void CommandHistory::addAlias(const std::string& alias,
                              const std::string& command) {
    impl_->addAlias(alias, command);
}

void CommandHistory::executeAlias(const std::string& alias) {
    impl_->executeAlias(alias);
}

void CommandHistory::deleteCommand(size_t index) {
    impl_->deleteCommand(index);
}

void CommandHistory::deleteCommands(const std::vector<size_t>& indices) {
    impl_->deleteCommands(indices);
}

void CommandHistory::sortHistoryByTime() { impl_->sortHistoryByTime(); }

void CommandHistory::printFrequencyReport() const {
    impl_->printFrequencyReport();
}

void CommandHistory::filterHistoryByTime(std::time_t startTime,
                                         std::time_t endTime) const {
    impl_->filterHistoryByTime(startTime, endTime);
}

void CommandHistory::clearHistory() { impl_->clearHistory(); }

void CommandHistory::exportHistory(const std::string& filePath) const {
    impl_->exportHistory(filePath);
}

int CommandHistory::getCommandFrequency(const std::string& command) const {
    return impl_->getCommandFrequency(command);
}

}  // namespace lithium::debug