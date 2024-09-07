#ifndef SERVER_CORE_HPP
#define SERVER_CORE_HPP

#include <memory>
#include <string>
#include "atom/async/message_bus.hpp"
#include "command.hpp"
#include "eventloop.hpp"
#include "executor.hpp"

class ServerCore {
public:
    ServerCore(size_t num_threads = std::thread::hardware_concurrency());
    ~ServerCore();

    void start();
    void stop();

    template <typename CommandType>
    void registerCommand(const std::string& commandName,
                         std::function<void(const CommandType&)> handler);

    template <typename CommandType>
    void executeCommand(const std::string& commandName,
                        const CommandType& command);

    template <typename MessageType>
    void subscribe(const std::string& topic,
                   std::function<void(const MessageType&)> handler);

    template <typename MessageType>
    void publish(const std::string& topic, const MessageType& message);

    void scheduleTask(
        std::function<void()> task,
        std::chrono::milliseconds delay = std::chrono::milliseconds(0));

    AsyncExecutor& getAsyncExecutor();
    EventLoop& getEventLoop();
    atom::async::MessageBus& getMessageBus();

private:
    std::unique_ptr<AsyncExecutor> asyncExecutor;
    std::shared_ptr<EventLoop> eventLoop;
    std::unique_ptr<CommandDispatcher> commandDispatcher;
    std::shared_ptr<atom::async::MessageBus> messageBus;

    void initializeSystemEvents();
};

#endif  // SERVER_CORE_HPP
