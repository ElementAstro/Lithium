#include "shared_component.hpp"

#include "atom/thread/thread.hpp"
#include "atom/server/message_bus.hpp"
#include "atom/type/iparams.hpp"

#include "atom/log/loguru.hpp"
#include "config.h"

SharedComponent::SharedComponent() : Component()
{
    DLOG_F(INFO, _("Shared component is created."));

    m_handleFunction = [this](std::shared_ptr<Message> message)
    {
        if (message)
        {
            switch (message->type())
            {
            case Message::Type::kText:
            {
                auto textMessage = std::dynamic_pointer_cast<TextMessage>(message);
                if (textMessage)
                {
                    DLOG_F(INFO, _("Text message is received: {}"), textMessage->value());
                    m_handleText->match(textMessage->name(),textMessage);
                }
                break;
            }
            case Message::Type::kNumber:
            {
                auto numberMessage = std::dynamic_pointer_cast<NumberMessage>(message);
                if (numberMessage)
                {
                    DLOG_F(INFO, _("Number message is received: {}"), numberMessage->value());
                    m_handleNumber->match(numberMessage->name(),numberMessage);
                }
            }
            case Message::Type::kBoolean:
            {
                auto booleanMessage = std::dynamic_pointer_cast<BooleanMessage>(message);
                if (booleanMessage)
                {
                    DLOG_F(INFO, _("Boolean message is received: {}"), booleanMessage->value());
                    m_handleBoolean->match(booleanMessage->name(),booleanMessage);
                }
                break;
            }
            case Message::Type::kParams:
            {
                auto paramsMessage = std::dynamic_pointer_cast<ParamsMessage>(message);
                if (paramsMessage)
                {
                    DLOG_F(INFO, _("Params message is received: {}"), paramsMessage->value()->toJson());
                    m_handleParams->match(paramsMessage->name(),paramsMessage);
                }
                break;
            }
            default:
                break;
            }
        } };

    // Initialize message handlers
    m_handleVoid = std::make_unique<Atom::Utils::StringSwitch<const std::shared_ptr<VoidMessage> &>>();
    m_handleText = std::make_unique<Atom::Utils::StringSwitch<const std::shared_ptr<TextMessage> &>>();
    m_handleNumber = std::make_unique<Atom::Utils::StringSwitch<const std::shared_ptr<NumberMessage> &>>();
    m_handleBoolean = std::make_unique<Atom::Utils::StringSwitch<const std::shared_ptr<BooleanMessage> &>>();
    m_handleParams = std::make_unique<Atom::Utils::StringSwitch<const std::shared_ptr<ParamsMessage> &>>();
}

SharedComponent::~SharedComponent()
{

    DLOG_F(INFO, _("Shared component is destroyed."));
}

bool SharedComponent::Initialize()
{
    m_handleVoid->registerCase("getVersion", [this](const std::shared_ptr<VoidMessage> &message)
                               {
        this->SendTextMessage("getVersion", GetVersion());
        return true; });
    m_handleVoid->registerCase("getName", [this](const std::shared_ptr<VoidMessage> &message)
                               {
        this->SendTextMessage("getName", GetName());
        return true; });
    m_handleVoid->registerCase("getPackageInfo", [this](const std::shared_ptr<VoidMessage> &message)
                                   {
        this->SendTextMessage("getPackageInfo", this->GetComponentInfo().dump());
        return true; });
    return true;
}

bool SharedComponent::Destroy()
{
    return true;
}

bool SharedComponent::NeedMessageBus()
{
    return true;
}

bool SharedComponent::InjectMessageBus(std::shared_ptr<Atom::Server::MessageBus> messageBus)
{
    m_MessageBus = messageBus;
    if (!m_MessageBus)
    {
        LOG_F(ERROR, _("Message bus is null."));
        return false;
    }
    DLOG_F(INFO, _("Message bus is injected."));
    return true;
}

bool SharedComponent::ConnectMessageBus()
{
    if (!m_MessageBus)
    {
        LOG_F(ERROR, _("Message bus is null."));
        return false;
    }
    m_MessageBus->Subscribe<std::shared_ptr<Message>>("lithium.app", m_handleFunction);
    DLOG_F(INFO, _("Message bus is connected."));
    return true;
}

bool SharedComponent::DisconnectMessageBus()
{
    if (!m_MessageBus)
    {
        LOG_F(ERROR, _("Message bus is null."));
        return false;
    }
    // There is a very severe bug in the message bus.
    // It will cause a crash when the message bus is disconnected.
    // How should we identify which connection is the one we want to disconnect?
    m_MessageBus->Unsubscribe<std::shared_ptr<Message>>("lithium.app",m_handleFunction);
    DLOG_F(INFO, _("Message bus is disconnected."));
    return true;
}

bool SharedComponent::SendTextMessage(const std::string &message, const std::string &text)
{
    if (!m_MessageBus)
    {
        LOG_F(ERROR, _("Message bus is null."));
        return false;
    }
    m_MessageBus->Publish<std::shared_ptr<Message>>(message, std::make_shared<TextMessage>(message, text, "lithium.app", GetName()));
    return true;
}

bool SharedComponent::SendNumberMessage(const std::string &message, const double &number)
{
    if (!m_MessageBus)
    {
        LOG_F(ERROR, _("Message bus is null."));
        return false;
    }
    m_MessageBus->Publish<std::shared_ptr<Message>>(message, std::make_shared<NumberMessage>(message, number, "lithium.app", GetName()));
    return true;
}

bool SharedComponent::SendBooleanMessage(const std::string &message, const bool &boolean)
{
    if (!m_MessageBus)
    {
        LOG_F(ERROR, _("Message bus is null."));
        return false;
    }
    m_MessageBus->Publish<std::shared_ptr<Message>>(message, std::make_shared<BooleanMessage>(message, boolean, "lithium.app", GetName()));
    return true;
}

bool SharedComponent::SendParamsMessage(const std::string &message, const json &params)
{
    if (!m_MessageBus)
    {
        LOG_F(ERROR, _("Message bus is null."));
        return false;
    }
    // m_MessageBus->Publish<std::shared_ptr<Message>>(message, std::make_shared<ParamsMessage>(message, params, "lithium.app", GetName()));
    return true;
}

bool SharedComponent::NeedThreadPool()
{
    return true;
}

bool SharedComponent::InjectThreadPool(std::shared_ptr<Atom::Async::ThreadManager> threadPool)
{
    if (!threadPool)
    {
        LOG_F(ERROR, _("Thread pool is null."));
        return false;
    }
    DLOG_F(INFO, _("Thread pool is injected."));
    return true;
}