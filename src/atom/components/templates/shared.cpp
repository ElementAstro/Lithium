#include "shared.hpp"

#include "atom/error/exception.hpp"
#include "atom/log/loguru.hpp"
#include "atom/server/message_bus.hpp"
#include "atom/type/message.hpp"
#include "atom/utils/string.hpp"

using namespace atom::error;

SharedComponent::SharedComponent(const std::string &name) : Component(name) {
    m_handleFunction = [shared_this =
                            shared_from_this()](const std::any &message) {
        LOG_F(INFO, "SharedComponent::handleFunction");
        if (message.has_value()) {
            if (message.type() == typeid(std::shared_ptr<VoidMessage>)) {
                shared_this->dispatch(
                    "handleVoid",
                    std::any_cast<std::shared_ptr<VoidMessage>>(message));
            } else if (message.type() ==
                       typeid(std::shared_ptr<NumberMessage>)) {
                shared_this->dispatch(
                    "handleNumber",
                    std::any_cast<std::shared_ptr<NumberMessage>>(message));
            } else if (message.type() == typeid(std::shared_ptr<TextMessage>)) {
                shared_this->dispatch(
                    "handleText",
                    std::any_cast<std::shared_ptr<TextMessage>>(message));
            } else if (message.type() ==
                       typeid(std::shared_ptr<BooleanMessage>)) {
                shared_this->dispatch(
                    "handleBoolean",
                    std::any_cast<std::shared_ptr<BooleanMessage>>(message));
            } else if (message.type() ==
                       typeid(std::shared_ptr<ParamsMessage>)) {
                shared_this->dispatch(
                    "handleParams",
                    std::any_cast<std::shared_ptr<ParamsMessage>>(message));
            } else {
                LOG_F(ERROR,
                      "SharedComponent::handleFunction: unknown message type");
                THROW_EXCEPTION(
                    "SharedComponent::handleFunction: unknown message type");
            }
        } else {
            LOG_F(ERROR, "SharedComponent::handleFunction: message is null");
            THROW_EXCEPTION("SharedComponent::handleFunction: message is null");
        }
    };

    registerCommand("handleNumber", handleNumberMessage);
    registerCommand("handleText", handleTextMessage);
    registerCommand("handleBoolean", handleBooleanMessage);
    registerCommand("handleVoid", handleVoidMessage);
}

SharedComponent::~SharedComponent() {}

bool SharedComponent::initialize() { return true; }

bool SharedComponent::destroy() { return true; }

void SharedComponent::handleVoidMessage(
    const std::shared_ptr<VoidMessage> &message) {
    LOG_F(INFO, "SharedComponent::handleVoid");
    auto name = message->name();
    try
    {
        
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    
}

void SharedComponent::handleTextMessage(
    const std::shared_ptr<TextMessage> &message) {
    LOG_F(INFO, "SharedComponent::handleText");
}

void SharedComponent::handleBooleanMessage(
    const std::shared_ptr<BooleanMessage> &message) {
    LOG_F(INFO, "SharedComponent::handleBoolean");
}

void SharedComponent::handleNumberMessage(
    const std::shared_ptr<NumberMessage> &message) {
    LOG_F(INFO, "SharedComponent::handleNumber");
}