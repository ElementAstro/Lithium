#include "command.hpp"

namespace lithium {
void CommandDispatcher::unregisterCommand(const CommandID& id) {
    std::unique_lock lock(mutex_);
    handlers_.erase(id);
    undoHandlers_.erase(id);
}

void CommandDispatcher::clearHistory() {
    std::unique_lock lock(mutex_);
    history_.clear();
}

std::vector<CommandDispatcher::CommandID> CommandDispatcher::getActiveCommands()
    const {
    std::shared_lock lock(mutex_);
    std::vector<CommandID> activeCommands;
    activeCommands.reserve(handlers_.size());
    for (const auto& [id, _] : handlers_) {
        activeCommands.push_back(id);
    }
    return activeCommands;
}

}  // namespace lithium
