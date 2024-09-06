#include "app.hpp"

#include <iostream>

ServerCore::ServerCore(size_t num_threads)
    : asyncExecutor(std::make_unique<AsyncExecutor>(num_threads)),
      eventLoop(std::make_shared<EventLoop>()),
      commandDispatcher(std::make_unique<CommandDispatcher>(eventLoop)),
      messageBus(atom::async::MessageBus::createShared()) {
    initializeSystemEvents();
}

ServerCore::~ServerCore() { stop(); }

void ServerCore::start() {
    publish("system.status", std::string("Server starting"));
    // Additional startup logic can be added here
    publish("system.status", std::string("Server started"));
}

void ServerCore::stop() {
    publish("system.status", std::string("Server stopping"));
    asyncExecutor->shutdown();
    eventLoop->stop();
    messageBus->clearAllSubscribers();
    publish("system.status", std::string("Server stopped"));
}

template <typename CommandType>
void ServerCore::registerCommand(
    const std::string& commandName,
    std::function<void(const CommandType&)> handler) {
    commandDispatcher->registerCommand<CommandType>(
        commandName, [this, handler, commandName](const CommandType& cmd) {
            publish("system.command.executed", commandName);
            handler(cmd);
        });
}

template <typename CommandType>
void ServerCore::executeCommand(const std::string& commandName,
                                const CommandType& command) {
    publish("system.command.executing", commandName);
    commandDispatcher->dispatch(commandName, command);
}

template <typename MessageType>
void ServerCore::subscribe(const std::string& topic,
                           std::function<void(const MessageType&)> handler) {
    messageBus->subscribe<MessageType>(topic, handler);
}

template <typename MessageType>
void ServerCore::publish(const std::string& topic, const MessageType& message) {
    messageBus->publish(topic, message);
}

void ServerCore::scheduleTask(std::function<void()> task,
                              std::chrono::milliseconds delay) {
    if (delay.count() == 0) {
        asyncExecutor->submit(task);
    } else {
        eventLoop->postDelayed(delay, std::move(task));
    }
}

AsyncExecutor& ServerCore::getAsyncExecutor() { return *asyncExecutor; }

EventLoop& ServerCore::getEventLoop() { return *eventLoop; }

atom::async::MessageBus& ServerCore::getMessageBus() { return *messageBus; }

void ServerCore::initializeSystemEvents() {
    subscribe<std::string>("system.status", [](const std::string& status) {
        std::cout << "System status: " << status << std::endl;
    });

    subscribe<std::string>(
        "system.command.executing", [](const std::string& commandName) {
            std::cout << "Executing command: " << commandName << std::endl;
        });

    subscribe<std::string>(
        "system.command.executed", [](const std::string& commandName) {
            std::cout << "Command executed: " << commandName << std::endl;
        });
}

// Explicit template instantiations for common types
template void ServerCore::registerCommand<std::string>(
    const std::string&, std::function<void(const std::string&)>);
template void ServerCore::executeCommand<std::string>(const std::string&,
                                                      const std::string&);
template void ServerCore::subscribe<std::string>(
    const std::string&, std::function<void(const std::string&)>);
template void ServerCore::publish<std::string>(const std::string&,
                                               const std::string&);