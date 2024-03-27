#include "atom/driver/camera.hpp"

#include <iostream>

class MySharedDriver : public AtomCamera {
public:
    explicit MySharedDriver(const std::string &name);
    virtual ~MySharedDriver();

    virtual bool initialize() override;
    virtual bool destroy() override;

protected:
    json helloWorld(const json &params);
};

MySharedDriver::MySharedDriver(const std::string &name)
    : AtomCamera(name) {
    LOG_F(INFO, "Load {}", name);

    initialize();

    registerFunc("helloWorld", &MySharedDriver::helloWorld, this);

    registerVariable("var_x", 0, "a test var");
}

MySharedDriver::~MySharedDriver() {}

bool MySharedDriver::initialize() {
    AtomCamera::initialize();
    return true;
}

bool MySharedDriver::destroy() {
    Component::destroy();
    return true;
}

json MySharedDriver::helloWorld(const json &params) {
    LOG_F(INFO, "helloWorld with {}", params.dump());
    return {};
}

int main() {
    std::shared_ptr<MySharedDriver> mycomponent =
        std::make_shared<MySharedDriver>("mycomponent");
    mycomponent->runFunc("helloWorld", {{"aaa", "aaaa"}});
    auto myvar = mycomponent->getVariable<int>("var_x");
    std::cout << (myvar.has_value() ? myvar.value() : -1) << std::endl;
    mycomponent->setVariable("var_x", 1);
    myvar = mycomponent->getVariable<int>("var_x");
    std::cout << (myvar.has_value() ? myvar.value() : -1) << std::endl;

    myvar = mycomponent->getVariable<double>("CCD_TEMPERATURE_VALUE");
    std::cout << (myvar.has_value() ? myvar.value() : -1) << std::endl;

    if (!mycomponent->setVariable("CCD_TEMPERATURE_VALUE", 10.0))
    {
        LOG_F(ERROR, "Failed to set temperature");
    }

    mycomponent->runFunc("startExposure", {{"exposure", 100}});

    mycomponent->runFunc(
        "registerVariable",
        {{"name", "status"}, {"value", "ok"}, {"description", "a test value"}});
    std::cout << mycomponent->getVariableInfo("status") << std::endl;
    return 0;
}