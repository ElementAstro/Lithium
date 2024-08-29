#ifndef LITHIUM_APP_COMMAND_HPP
#define LITHIUM_APP_COMMAND_HPP

#include <any>
#include <functional>
#include <future>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace lithium {
class CommandDispatcher {
public:
    using CommandID = std::string;
    using CommandHandler = std::function<void(const std::any&)>;
    using ResultType = std::variant<std::any, std::exception_ptr>;
    using CommandCallback =
        std::function<void(const CommandID&, const ResultType&)>;

    // 注册命令
    template <typename CommandType>
    void registerCommand(
        const CommandID& id, CommandHandler handler,
        std::optional<CommandHandler> undoHandler = std::nullopt) {
        std::unique_lock lock(mutex_);
        handlers_[id] = [handler](const std::any& cmd) {
            handler(std::any_cast<const CommandType&>(cmd));
        };
        if (undoHandler) {
            undoHandlers_[id] = [undoHandler](const std::any& cmd) {
                (*undoHandler)(std::any_cast<const CommandType&>(cmd));
            };
        }
    }

    void unregisterCommand(const CommandID& id);

    template <typename CommandType>
    auto dispatch(
        const CommandID& id, const CommandType& command, bool async = true,
        std::optional<std::chrono::milliseconds> delay = std::nullopt,
        CommandCallback callback = nullptr) -> std::future<ResultType> {
        auto task = [this, id, command, callback]() -> ResultType {
            try {
                std::shared_lock lock(mutex_);
                auto it = handlers_.find(id);
                if (it != handlers_.end()) {
                    it->second(command);
                    recordHistory(id, command);
                    ResultType result = command;
                    if (callback)
                        callback(id, result);
                    return result;
                } else {
                    throw std::runtime_error("Command not found: " + id);
                }
            } catch (...) {
                auto ex = std::current_exception();
                if (callback) {
                    callback(id, ex);
                }
                return ex;
            }
        };

        if (delay) {
            return std::async(std::launch::async, [task, delay]() {
                std::this_thread::sleep_for(*delay);
                return task();
            });
        }
        if (async) {
            return std::async(std::launch::async, task);
        }
        return std::async(std::launch::deferred, task);
    }

    template <typename CommandType>
    auto getResult(std::future<ResultType>& resultFuture) -> CommandType {
        auto result = resultFuture.get();
        if (std::holds_alternative<std::any>(result)) {
            return std::any_cast<CommandType>(std::get<std::any>(result));
        } else {
            std::rethrow_exception(std::get<std::exception_ptr>(result));
        }
    }

    template <typename CommandType>
    void undo(const CommandID& id, const CommandType& command) {
        std::unique_lock lock(mutex_);
        auto it = undoHandlers_.find(id);
        if (it != undoHandlers_.end()) {
            it->second(command);
        }
    }

    template <typename CommandType>
    void redo(const CommandID& id, const CommandType& command) {
        dispatch(id, command, false).get();
    }

    // 获取命令执行历史
    template <typename CommandType>
    auto getCommandHistory(const CommandID& id) -> std::vector<CommandType> {
        std::shared_lock lock(mutex_);
        std::vector<CommandType> history;
        if (auto it = history_.find(id); it != history_.end()) {
            for (const auto& cmd : it->second) {
                history.push_back(std::any_cast<CommandType>(cmd));
            }
        }
        return history;
    }

    void clearHistory();

    template <typename... CommandTypes>
    void registerCommandChain(const std::vector<CommandID>& ids,
                              std::tuple<CommandTypes...> commands) {
        std::apply(
            [this, &ids, &commands](auto&&... cmds) {
                (..., dispatch(ids[&cmds - &std::get<0>(commands)], cmds));
            },
            commands);
    }

    auto getActiveCommands() const -> std::vector<CommandID>;

private:
    void recordHistory(const CommandID& id, const std::any& command) {
        history_[id].push_back(command);
        if (history_[id].size() > maxHistorySize_) {
            history_[id].erase(history_[id].begin());
        }
    }

    std::unordered_map<CommandID, CommandHandler> handlers_;
    std::unordered_map<CommandID, CommandHandler> undoHandlers_;
    std::unordered_map<CommandID, std::vector<std::any>> history_;
    mutable std::timed_mutex mutex_;
    std::size_t maxHistorySize_ = 100;
};
}  // namespace lithium

#endif