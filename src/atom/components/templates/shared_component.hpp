#pragma once

#include "atom/components/component.hpp"
#include "atom/type/message.hpp"
#include "atom/utils/switch.hpp"

class Atom::Server::MessageBus;
class Atom::Async::ThreadManager;

class SharedComponent : public Component
{
public:
    SharedComponent();
    ~SharedComponent();

    // -------------------------------------------------------------------
    // Common methods
    // -------------------------------------------------------------------

    bool Initialize() override;
    bool Destroy() override;

    // -------------------------------------------------------------------
    // Message methods
    // -------------------------------------------------------------------

    bool NeedMessageBus();
    bool InjectMessageBus(std::shared_ptr<Atom::Server::MessageBus> messageBus);
    bool ConnectMessageBus();
    bool DisconnectMessageBus();
    bool SendTextMessage(const std::string &message, const std::string &text);
    bool SendNumberMessage(const std::string &message, const double &number);
    bool SendBooleanMessage(const std::string &message, const bool &boolean);
    bool SendParamsMessage(const std::string &message, const json &params);

    // -------------------------------------------------------------------
    // Handbler methods
    // -------------------------------------------------------------------

    void SetHandleFunction(std::function<void(std::shared_ptr<Message>)> handleFunction)
    {
        m_handleFunction = handleFunction;
    }

    void RegisterFunc

    // -------------------------------------------------------------------
    // Thread methods
    // -------------------------------------------------------------------

    bool NeedThreadPool();
    bool InjectThreadPool(std::shared_ptr<Atom::Async::ThreadManager> threadPool);

private:
    // This is a little bit hacky, but it works.
    std::shared_ptr<Atom::Server::MessageBus> m_MessageBus;
    std::shared_ptr<Atom::Async::ThreadManager> m_ThreadPool;

    std::function<void(std::shared_ptr<Message>)> m_handleFunction;

    // Message handlers
    std::unique_ptr<Atom::Utils::StringSwitch<const std::shared_ptr<VoidMessage> &>> m_handleVoid;
    std::unique_ptr<Atom::Utils::StringSwitch<const std::shared_ptr<TextMessage> &>> m_handleText;
    std::unique_ptr<Atom::Utils::StringSwitch<const std::shared_ptr<NumberMessage> &>> m_handleNumber;
    std::unique_ptr<Atom::Utils::StringSwitch<const std::shared_ptr<BooleanMessage> &>> m_handleBoolean;
    std::unique_ptr<Atom::Utils::StringSwitch<const std::shared_ptr<ParamsMessage> &>> m_handleParams;

};