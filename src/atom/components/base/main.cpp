#include "atom/components/templates/shared_component.hpp"

#include <iostream>

class MySharedComponent : public SharedComponent {
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

    registerFunc("helloWorld", &MySharedComponent::helloWorld, this);

    registerVariable("var_x", 0, "a test var");
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
    mycomponent->runFunc("helloWorld", {{"aaa", "aaaa"}});
    auto myvar = mycomponent->getVariable<int>("var_x");
    std::cout << (myvar.has_value() ? myvar.value() : -1) << std::endl;
    mycomponent->setVariable("var_x", 1);
    myvar = mycomponent->getVariable<int>("var_x");
    std::cout << (myvar.has_value() ? myvar.value() : -1) << std::endl;

    mycomponent->runFunc(
        "registerVariable",
        {{"name", "status"}, {"value", "ok"}, {"description", "a test value"}});
    std::cout << mycomponent->getVariableInfo("status") << std::endl;
    return 0;
}