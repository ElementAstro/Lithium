#include "atom/components/templates/shared.hpp"

#include <iostream>

#include "atom/log/loguru.hpp"

class MySharedComponent : public Component<MySharedComponent> {
public:
    explicit MySharedComponent(const std::string &name);
    virtual ~MySharedComponent();

    virtual bool initialize() override;
    virtual bool destroy() override;

protected:
    json helloWorld(const json &params);
};

MySharedComponent::MySharedComponent(const std::string &name)
    : Component<MySharedComponent>(name) {
    LOG_F(INFO, "Load {}", name);

    initialize();

    registerCommand("helloWorld", &MySharedComponent::helloWorld, PointerSentinel(shared_from_this()));
}

MySharedComponent::~MySharedComponent() {}

bool MySharedComponent::initialize() {
    return true;
}

bool MySharedComponent::destroy() {
    return true;
}

json MySharedComponent::helloWorld(const json &params) {
    LOG_F(INFO, "helloWorld with {}", params.dump());
    return {};
}

int main() {
    std::shared_ptr<MySharedComponent> mycomponent =
        std::make_shared<MySharedComponent>("mycomponent");
    mycomponent->dispatch("helloWorld", json::object({"message", "hello"}));
    return 0;
}