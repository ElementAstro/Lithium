#include "shared_component.hpp"

#include "atom/log/loguru.hpp"
#include "atom/server/message_bus.hpp"
#include "atom/utils/string.hpp"

//#include "config.h"

#include "macro.hpp"

#define GET_ARGUMENT_S(command, type, name)                                 \
    if (!args.get<type>(#name).has_value()) {                               \
        this->SendTextMessage(#command,                                     \
                              fmt::format("Missing arguments: {}", #name)); \
        return;                                                             \
    }                                                                       \
    type name = args.get<type>(#name).value();

SharedComponent::SharedComponent(const std::string &name) : Component(name) {
    DLOG_F(INFO, "Shared component is created.");
}

SharedComponent::~SharedComponent() {
    DLOG_F(INFO, "Shared component is destroyed.");
}

bool SharedComponent::initialize() {
    Component::initialize();
    DLOG_F(INFO, "Shared component is initializing ...");

    m_handleFunction = [this](std::shared_ptr<Message> message) {
        if (message) {
            switch (message->type()) {
                case Message::Type::kText: {
                    auto textMessage =
                        std::dynamic_pointer_cast<TextMessage>(message);
                    if (textMessage) {
                        DLOG_F(INFO, "Text message is received: {}",
                               textMessage->value());
                        m_handleText->match(textMessage->name(), textMessage);
                    }
                    break;
                }
                case Message::Type::kNumber: {
                    auto numberMessage =
                        std::dynamic_pointer_cast<NumberMessage>(message);
                    if (numberMessage) {
                        DLOG_F(INFO, "Number message is received: {}",
                               numberMessage->value());
                        m_handleNumber->match(numberMessage->name(),
                                              numberMessage);
                    }
                }
                case Message::Type::kBoolean: {
                    auto booleanMessage =
                        std::dynamic_pointer_cast<BooleanMessage>(message);
                    if (booleanMessage) {
                        DLOG_F(INFO, "Boolean message is received: {}",
                               booleanMessage->value());
                        m_handleBoolean->match(booleanMessage->name(),
                                               booleanMessage);
                    }
                    break;
                }
                case Message::Type::kParams: {
                    auto paramsMessage =
                        std::dynamic_pointer_cast<ParamsMessage>(message);
                    if (paramsMessage) {
                        DLOG_F(INFO, "Params message is received: {}",
                               paramsMessage->value().toJson());
                        m_handleParams->match(paramsMessage->name(),
                                              paramsMessage);
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };

    // Initialize message handlers
    m_handleVoid = std::make_unique<
        Atom::Utils::StringSwitch<const std::shared_ptr<VoidMessage> &>>();
    m_handleText = std::make_unique<
        Atom::Utils::StringSwitch<const std::shared_ptr<TextMessage> &>>();
    m_handleNumber = std::make_unique<
        Atom::Utils::StringSwitch<const std::shared_ptr<NumberMessage> &>>();
    m_handleBoolean = std::make_unique<
        Atom::Utils::StringSwitch<const std::shared_ptr<BooleanMessage> &>>();
    m_handleParams = std::make_unique<
        Atom::Utils::StringSwitch<const std::shared_ptr<ParamsMessage> &>>();

    DLOG_F(INFO, "Shared component is initialized");
    return true;
}

bool SharedComponent::destroy() { 
    Component::destroy();
    return true; }

bool SharedComponent::NeedMessageBus() { return true; }

bool SharedComponent::InjectMessageBus(
    std::shared_ptr<Atom::Server::MessageBus> messageBus) {
    m_MessageBus = messageBus;
    if (!m_MessageBus) {
        LOG_F(ERROR, "Message bus is null.");
        return false;
    }
    DLOG_F(INFO, "Message bus is injected.");
    return true;
}

bool SharedComponent::ConnectMessageBus() {
    if (!m_MessageBus) {
        LOG_F(ERROR, "Message bus is null.");
        return false;
    }
    m_MessageBus->Subscribe<std::shared_ptr<Message>>("lithium.app",
                                                      m_handleFunction);
    DLOG_F(INFO, "Message bus is connected.");
    return true;
}

bool SharedComponent::DisconnectMessageBus() {
    if (!m_MessageBus) {
        LOG_F(ERROR, "Message bus is null.");
        return false;
    }
    // There is a very severe bug in the message bus.
    // It will cause a crash when the message bus is disconnected.
    // How should we identify which connection is the one we want to disconnect?
    m_MessageBus->Unsubscribe<std::shared_ptr<Message>>("lithium.app",
                                                        m_handleFunction);
    DLOG_F(INFO, "Message bus is disconnected.");
    return true;
}

bool SharedComponent::SendTextMessage(const std::string &message,
                                      const std::string &text) {
    if (!m_MessageBus) {
        LOG_F(ERROR, "Message bus is null.");
        return false;
    }
    m_MessageBus->Publish<std::shared_ptr<Message>>(
        message,
        std::make_shared<TextMessage>(message, text, "lithium.app", getName()));
    return true;
}

bool SharedComponent::SendNumberMessage(const std::string &message,
                                        const double &number) {
    if (!m_MessageBus) {
        LOG_F(ERROR, "Message bus is null.");
        return false;
    }
    m_MessageBus->Publish<std::shared_ptr<Message>>(
        message, std::make_shared<NumberMessage>(message, number, "lithium.app",
                                                 getName()));
    return true;
}

bool SharedComponent::SendBooleanMessage(const std::string &message,
                                         const bool &boolean) {
    if (!m_MessageBus) {
        LOG_F(ERROR, "Message bus is null.");
        return false;
    }
    m_MessageBus->Publish<std::shared_ptr<Message>>(
        message, std::make_shared<BooleanMessage>(message, boolean,
                                                  "lithium.app", getName()));
    return true;
}

bool SharedComponent::SendParamsMessage(const std::string &message,
                                        const Args &params) {
    if (!m_MessageBus) {
        LOG_F(ERROR, "Message bus is null.");
        return false;
    }
    m_MessageBus->Publish<std::shared_ptr<Message>>(
        message, std::make_shared<ParamsMessage>(message, params, "lithium.app",
                                                 getName()));
    return true;
}
