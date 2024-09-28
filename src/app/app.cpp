#include "app.hpp"
#include <vector>

#include "exception.hpp"
#include "executor.hpp"
#include "message_bus.hpp"
#include "token.hpp"
#include "utils/constant.hpp"

#include "atom/function/global_ptr.hpp"
#include "atom/log/loguru.hpp"
#include "atom/type/json.hpp"

namespace lithium::app {

ServerCore::ServerCore(size_t num_threads) {
    LOG_F(INFO, "Initializing ServerCore with {} threads", num_threads);
    GET_OR_CREATE_PTR(eventLoop, EventLoop, Constants::EVENTLOOP);
    GET_OR_CREATE_PTR(componentManager, ComponentManager,
                      Constants::COMPONENT_MANAGER);
    GET_OR_CREATE_PTR_THIS(commandDispatcher, CommandDispatcher,
                           Constants::DISPATCHER,
                           std::make_unique<CommandDispatcher>(eventLoop));
    GET_OR_CREATE_PTR(stringSplitter, StringSplitter,
                      Constants::STRING_SPLITTER);
    GET_OR_CREATE_PTR(messageBus, atom::async::MessageBus,
                      Constants::MESSAGE_BUS);
    GET_OR_CREATE_PTR(asyncExecutor, AsyncExecutor, Constants::EXECUTOR);

    initializeSystemEvents();
    LOG_F(INFO, "ServerCore initialized");
}

ServerCore::~ServerCore() {
    LOG_F(INFO, "Destroying ServerCore");
    stop();
    LOG_F(INFO, "ServerCore destroyed");
}

void ServerCore::start() {
    LOG_F(INFO, "Starting ServerCore");
    publish("system.status", std::string("Server starting"));

    // 加载所有组件
    auto components = componentManager->getComponentList();
    for (const auto& component : components) {
        json params;  // You can customize parameters if needed
        componentManager->loadComponent(params);
        LOG_F(INFO, "Loaded component: {}", component);
    }

    publish("system.status", std::string("Server started"));
    LOG_F(INFO, "ServerCore started");
}

void ServerCore::stop() {
    LOG_F(INFO, "Stopping ServerCore");
    publish("system.status", std::string("Server stopping"));

    // 卸载所有组件
    auto components = componentManager->getComponentList();
    for (const auto& component : components) {
        json params;  // You can customize parameters if needed
        componentManager->unloadComponent(params);
        LOG_F(INFO, "Unloaded component: {}", component);
    }

    componentManager->destroy();  // 销毁组件管理器
    LOG_F(INFO, "ComponentManager destroyed");

    asyncExecutor->shutdown();
    LOG_F(INFO, "AsyncExecutor shutdown");

    eventLoop->stop();
    LOG_F(INFO, "EventLoop stopped");

    messageBus->clearAllSubscribers();
    LOG_F(INFO, "MessageBus cleared all subscribers");

    publish("system.status", std::string("Server stopped"));
    LOG_F(INFO, "ServerCore stopped");
}

template <typename CommandType>
void ServerCore::registerCommand(
    const std::string& commandName,
    std::function<void(const CommandType&)> handler) {
    LOG_F(INFO, "Registering command: {}", commandName);
    commandDispatcher->registerCommand<CommandType>(
        commandName, [this, handler, commandName](const CommandType& cmd) {
            publish("system.command.executed", commandName);
            handler(cmd);
            LOG_F(INFO, "Command executed: {}", commandName);
        });
}

template <typename CommandType>
void ServerCore::executeCommand(const std::string& commandName,
                                const CommandType& command) {
    LOG_F(INFO, "Executing command: {}", commandName);
    // Check if the command is valid
    std::set<char> delimeters = {".", " "};
    if (auto cmd = stringSplitter->splitAndValidate(commandName, delimeters);
        !cmd.empty()) {
        if (cmd.size() == 3) {
            if (!componentManager->hasComponent(cmd[0])) {
                THROW_INVALID_COMPONENT("Invalid module: ", cmd[0]);
            }
            if (auto component = componentManager->getComponent(commandName);
                !component.has_value()) {
                THROW_OBJ_NOT_EXIST("Command not found: ", commandName);
            } else {
                auto componentPtr = component.value();
                if (componentPtr.expired()) {
                    THROW_OBJ_UNINITIALIZED("Command not initialized: ",
                                            commandName);
                }
                if (!componentPtr.lock()->has(commandName)) {
                    THROW_INVALID_COMMAND("Invalid command: ", commandName);
                }
                std::vector<std::any> params;
                if constexpr (std::is_same_v<CommandType, json>) {
                    for (const auto& arg : command) {
                        params.push_back(arg);
                    }
                    componentPtr.lock()->dispatch(commandName, params);
                } else if constexpr (std::is_same_v<CommandType, std::string>) {
                    componentPtr.lock()->dispatch(commandName, command);
                } else {
                    componentPtr.lock()->dispatch(commandName, command);
                }
            }
        }
        THROW_INVALID_COMMAND("Invalid command arguments size: ", cmd.size(),
                              " for command: ", commandName);
    }
    publish("system.command.executing", commandName);
    commandDispatcher->dispatch(commandName, command);
    LOG_F(INFO, "Command dispatched: {}", commandName);
}

template <typename MessageType>
void ServerCore::subscribe(const std::string& topic,
                           std::function<void(const MessageType&)> handler) {
    LOG_F(INFO, "Subscribing to topic: {}", topic);
    messageBus->subscribe<MessageType>(topic, handler);
}

template <typename MessageType>
void ServerCore::publish(const std::string& topic, const MessageType& message) {
    LOG_F(INFO, "Publishing message to topic: {}", topic);
    messageBus->publish(topic, message);
}

void ServerCore::scheduleTask(std::function<void()> task,
                              std::chrono::milliseconds delay) {
    if (delay.count() == 0) {
        asyncExecutor->submit(task);
        LOG_F(INFO, "Task submitted immediately");
    } else {
        eventLoop->postDelayed(delay, std::move(task));
        LOG_F(INFO, "Task scheduled with delay: {} ms", delay.count());
    }
}

AsyncExecutor& ServerCore::getAsyncExecutor() { return *asyncExecutor; }

EventLoop& ServerCore::getEventLoop() { return *eventLoop; }

atom::async::MessageBus& ServerCore::getMessageBus() { return *messageBus; }

void ServerCore::initializeSystemEvents() {
    subscribe<std::string>("system.status", [](const std::string& status) {
        LOG_F(INFO, "System status: {}", status);
    });

    subscribe<std::string>(
        "system.command.executing", [](const std::string& commandName) {
            LOG_F(INFO, "Executing command: {}", commandName);
        });

    subscribe<std::string>("system.command.executed",
                           [](const std::string& commandName) {
                               LOG_F(INFO, "Command executed: {}", commandName);
                           });
}

// Explicit template instantiations for common types
template void ServerCore::registerCommand<std::string>(
    const std::string&, std::function<void(const std::string&)>);
template void ServerCore::executeCommand<std::string>(const std::string&,
                                                      const std::string&);
template void ServerCore::executeCommand<json>(const std::string&, const json&);
template void ServerCore::executeCommand<std::vector<std::any>>(
    const std::string&, const std::vector<std::any>&);
template void ServerCore::subscribe<std::string>(
    const std::string&, std::function<void(const std::string&)>);
template void ServerCore::publish<std::string>(const std::string&,
                                               const std::string&);

}  // namespace lithium::app