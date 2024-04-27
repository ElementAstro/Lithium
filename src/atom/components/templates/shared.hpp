#ifndef ATOM_COMPONENT_SHARED_HPP
#define ATOM_COMPONENT_SHARED_HPP

#include "atom/components/component.hpp"

#include "atom/type/message.hpp"

class SharedComponent : public Component {
    explicit SharedComponent(const std::string &name);
    virtual ~SharedComponent() override;

    virtual bool initialize() override;
    virtual bool destroy() override;

    void handleVoidMessage(const std::shared_ptr<VoidMessage> &message);
    void handleTextMessage(const std::shared_ptr<TextMessage> &message);
    void handleNumberMessage(const std::shared_ptr<NumberMessage> &message);
    void handleBooleanMessage(const std::shared_ptr<BooleanMessage> &message);

private:
    std::function<void(const std::any &)> m_handleFunction;
};

#endif