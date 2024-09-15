#include "app.hpp"

#include "atom/function/global_ptr.hpp"
#include "exception.hpp"

#include "utils/constant.hpp"

#include "atom/type/json.hpp"

namespace lithium::app {
ServerCore::ServerCore(size_t num_threads)
    : asyncExecutor(std::make_unique<AsyncExecutor>(num_threads)),
      commandDispatcher(std::make_unique<CommandDispatcher>(eventLoop)),
      messageBus(atom::async::MessageBus::createShared()) {
    // Create EventLoop

    GET_OR_CREATE_PTR(eventLoop, EventLoop, Constants::EVENTLOOP);
    GET_OR_CREATE_PTR(componentManager, ComponentManager,
                      Constants::COMPONENT_MANAGER);
    GET_OR_CREATE_PTR(commandDispatcher, CommandDispatcher, Constants::DISPATCHER);
    
    initializeSystemEvents();

    
}

ServerCore::~ServerCore() { stop(); }

void ServerCore::start() {
    publish("system.status", std::string("Server starting"));

    // 加载所有组件
    auto components = componentManager->getComponentList();
    for (const auto& component : components) {
        json params;  // You can customize parameters if needed
        componentManager->loadComponent(params);
    }

    publish("system.status", std::string("Server started"));
}

void ServerCore::stop() {
    publish("system.status", std::string("Server stopping"));

    // 卸载所有组件
    auto components = componentManager->getComponentList();
    for (const auto& component : components) {
        json params;  // You can customize parameters if needed
        componentManager->unloadComponent(params);
    }

    componentManager->destroy();  // 销毁组件管理器

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

}  // namespace lithium::app
