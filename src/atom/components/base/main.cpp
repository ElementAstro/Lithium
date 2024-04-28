#include "atom/components/templates/shared.hpp"

#include <iostream>

class MySharedComponent : public SharedComponent<MySharedComponent> {
public:
    explicit MySharedComponent(const std::string &name);
    virtual ~MySharedComponent();

    virtual bool initialize() override;
    virtual bool destroy() override;

protected:
    json helloWorld(const json &params);
};

MySharedComponent::MySharedComponent(const std::string &name)
    : SharedComponent(name) {
    LOG_F(INFO, "Load {}", name);

    initialize();

    registerCommand("helloWorld",
                    [this](const json &params) { return helloWorld(params); });
}

MySharedComponent::~MySharedComponent() {}

bool MySharedComponent::initialize() {
    SharedComponent::initialize();
    return true;
}

bool MySharedComponent::destroy() {
    Component::destroy();
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