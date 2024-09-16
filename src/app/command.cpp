#include "command.hpp"
#include "eventloop.hpp"

#include "atom/log/loguru.hpp"

namespace lithium::app {
CommandDispatcher::CommandDispatcher(std::shared_ptr<EventLoop> eventLoop)
    : eventLoop_(std::move(eventLoop)) {
    LOG_F(INFO, "CommandDispatcher initialized");
}

void CommandDispatcher::unregisterCommand(const CommandID& id) {
    std::unique_lock lock(mutex_);
    handlers_.erase(id);
    undoHandlers_.erase(id);
    LOG_F(INFO, "Unregistered command: {}", id);
}

void CommandDispatcher::recordHistory(const CommandID& id,
                                      const std::any& command) {
    auto& commandHistory = history_[id];
    commandHistory.push_back(command);
    if (commandHistory.size() > maxHistorySize_) {
        commandHistory.erase(commandHistory.begin());
    }
    LOG_F(INFO, "Recorded history for command: {}", id);
}

void CommandDispatcher::notifySubscribers(const CommandID& id,
                                          const std::any& command) {
    auto it = subscribers_.find(id);
    if (it != subscribers_.end()) {
        for (auto& [_, callback] : it->second) {
            callback(id, command);
        }
        LOG_F(INFO, "Notified subscribers for command: {}", id);
    }
}

int CommandDispatcher::subscribe(const CommandID& id, EventCallback callback) {
    std::unique_lock lock(mutex_);
    int token = nextSubscriberId_++;
    subscribers_[id][token] = std::move(callback);
    LOG_F(INFO, "Subscribed to command: {} with token: {}", id, token);
    return token;
}

void CommandDispatcher::unsubscribe(const CommandID& id, int token) {
    std::unique_lock lock(mutex_);
    auto& callbacks = subscribers_[id];
    callbacks.erase(token);
    if (callbacks.empty()) {
        subscribers_.erase(id);
    }
    LOG_F(INFO, "Unsubscribed from command: {} with token: {}", id, token);
}

void CommandDispatcher::clearHistory() {
    std::unique_lock lock(mutex_);
    history_.clear();
    LOG_F(INFO, "Cleared all command history");
}

void CommandDispatcher::clearCommandHistory(const CommandID& id) {
    std::unique_lock lock(mutex_);
    history_.erase(id);
    LOG_F(INFO, "Cleared history for command: {}", id);
}

auto CommandDispatcher::getActiveCommands() const -> std::vector<CommandID> {
    std::shared_lock lock(mutex_);
    std::vector<CommandID> activeCommands;
    activeCommands.reserve(handlers_.size());
    for (const auto& [id, _] : handlers_) {
        activeCommands.push_back(id);
    }
    LOG_F(INFO, "Retrieved active commands");
    return activeCommands;
}

// Template implementations
template <typename CommandType>
void CommandDispatcher::registerCommand(
    const CommandID& id, std::function<void(const CommandType&)> handler,
    std::optional<std::function<void(const CommandType&)>> undoHandler) {
    std::unique_lock lock(mutex_);
    handlers_[id] = [handler](const std::any& cmd) {
        handler(std::any_cast<const CommandType&>(cmd));
    };
    if (undoHandler) {
        undoHandlers_[id] = [undoHandler](const std::any& cmd) {
            (*undoHandler)(std::any_cast<const CommandType&>(cmd));
        };
    }
    LOG_F(INFO, "Registered command: {}", id);
}

template <typename CommandType>
auto CommandDispatcher::dispatch(
    const CommandID& id, const CommandType& command, int priority,
    std::optional<std::chrono::milliseconds> delay,
    CommandCallback callback) -> std::future<ResultType> {
    LOG_F(INFO, "Dispatching command: {} with priority: {}", id, priority);
    auto task = [this, id, command, callback]() -> ResultType {
        try {
            std::shared_lock lock(mutex_);
            auto it = handlers_.find(id);
            if (it != handlers_.end()) {
                it->second(command);
                recordHistory(id, command);
                notifySubscribers(id, command);
                ResultType result = command;
                if (callback)
                    callback(id, result);
                LOG_F(INFO, "Command {} executed successfully", id);
                return result;
            } else {
                throw std::runtime_error("Command not found: " + id);
            }
        } catch (...) {
            auto ex = std::current_exception();
            if (callback) {
                callback(id, ex);
            }
            LOG_F(ERROR, "Command {} execution failed", id);
            return ex;
        }
    };

    if (delay) {
        return eventLoop_->postDelayed(*delay, priority, std::move(task));
    } else {
        return eventLoop_->post(priority, std::move(task));
    }
}

template <typename CommandType>
auto CommandDispatcher::getResult(std::future<ResultType>& resultFuture)
    -> CommandType {
    LOG_F(INFO, "Getting result for command");
    auto result = resultFuture.get();
    if (std::holds_alternative<std::any>(result)) {
        return std::any_cast<CommandType>(std::get<std::any>(result));
    } else {
        std::rethrow_exception(std::get<std::exception_ptr>(result));
    }
}

template <typename CommandType>
void CommandDispatcher::undo(const CommandID& id, const CommandType& command) {
    std::unique_lock lock(mutex_);
    auto it = undoHandlers_.find(id);
    if (it != undoHandlers_.end()) {
        it->second(command);
        LOG_F(INFO, "Undo command: {}", id);
    }
}

template <typename CommandType>
void CommandDispatcher::redo(const CommandID& id, const CommandType& command) {
    LOG_F(INFO, "Redo command: {}", id);
    dispatch(id, command, 0, std::nullopt).get();
}

template <typename CommandType>
auto CommandDispatcher::getCommandHistory(const CommandID& id)
    -> std::vector<CommandType> {
    std::shared_lock lock(mutex_);
    std::vector<CommandType> history;
    if (auto it = history_.find(id); it != history_.end()) {
        for (const auto& cmd : it->second) {
            history.push_back(std::any_cast<CommandType>(cmd));
        }
    }
    LOG_F(INFO, "Retrieved command history for: {}", id);
    return history;
}

}  // namespace lithium::app
