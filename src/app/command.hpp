#ifndef LITHIUM_APP_COMMAND_HPP
#define LITHIUM_APP_COMMAND_HPP

#include <any>
#include <functional>
#include <future>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace lithium::app {
class EventLoop;

class CommandDispatcher {
public:
    using CommandID = std::string;
    using CommandHandler = std::function<void(const std::any&)>;
    using ResultType = std::variant<std::any, std::exception_ptr>;
    using CommandCallback =
        std::function<void(const CommandID&, const ResultType&)>;
    using EventCallback =
        std::function<void(const CommandID&, const std::any&)>;

    explicit CommandDispatcher(std::shared_ptr<EventLoop> eventLoop);

    template <typename CommandType>
    void registerCommand(const CommandID& id,
                         std::function<void(const CommandType&)> handler,
                         std::optional<std::function<void(const CommandType&)>>
                             undoHandler = std::nullopt);

    void unregisterCommand(const CommandID& id);

    template <typename CommandType>
    auto dispatch(
        const CommandID& id, const CommandType& command, int priority = 0,
        std::optional<std::chrono::milliseconds> delay = std::nullopt,
        CommandCallback callback = nullptr) -> std::future<ResultType>;

    template <typename CommandType>
    auto getResult(std::future<ResultType>& resultFuture) -> CommandType;

    template <typename CommandType>
    void undo(const CommandID& id, const CommandType& command);

    template <typename CommandType>
    void redo(const CommandID& id, const CommandType& command);

    int subscribe(const CommandID& id, EventCallback callback);
    void unsubscribe(const CommandID& id, int token);

    template <typename CommandType>
    auto getCommandHistory(const CommandID& id) -> std::vector<CommandType>;

    void clearHistory();
    void clearCommandHistory(const CommandID& id);
    auto getActiveCommands() const -> std::vector<CommandID>;

private:
    void recordHistory(const CommandID& id, const std::any& command);
    void notifySubscribers(const CommandID& id, const std::any& command);

    std::unordered_map<CommandID, CommandHandler> handlers_;
    std::unordered_map<CommandID, CommandHandler> undoHandlers_;
    std::unordered_map<CommandID, std::vector<std::any>> history_;
    std::unordered_map<CommandID, std::unordered_map<int, EventCallback>>
        subscribers_;
    mutable std::shared_mutex mutex_;
    std::size_t maxHistorySize_ = 100;
    std::shared_ptr<EventLoop> eventLoop_;
    int nextSubscriberId_ = 0;
};
}  // namespace lithium::app

#endif  // LITHIUM_APP_COMMAND_HPP
