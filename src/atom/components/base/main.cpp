#include "atom/components/component.hpp"

#include <iostream>

#include "atom/log/loguru.hpp"

class MySharedComponent : public Component {
public:
    explicit MySharedComponent(const std::string &name);
    virtual ~MySharedComponent();

    virtual bool initialize() override;
    virtual bool destroy() override;

protected:
    void helloWorld(const std::string &params);

    void calc(int a, int b);

    int process(const std::vector<int> &params);
};

MySharedComponent::MySharedComponent(const std::string &name)
    : Component(name) {
    LOG_F(INFO, "Load {}", name);

    initialize();

    def("helloWorld", &MySharedComponent::helloWorld,
                    PointerSentinel(this));
    def("calc", &MySharedComponent::calc, PointerSentinel(this));
    def("process", &MySharedComponent::process,
                    PointerSentinel(this));
}

MySharedComponent::~MySharedComponent() {
    LOG_F(INFO, "Unload {}", getName());
}

bool MySharedComponent::initialize() { return true; }

bool MySharedComponent::destroy() { return true; }

void MySharedComponent::helloWorld(const std::string &params) {
    std::cout << "Hello " << params << std::endl;
}

void MySharedComponent::calc(int a, int b) { std::cout << a + b << std::endl; }

int MySharedComponent::process(const std::vector<int> &params) {
    int sum = 0;
    for (auto &param : params) {
        sum += param;
    }
    return sum;
}

class MyOtherSharedComponent : public Component {
public:
    explicit MyOtherSharedComponent(const std::string &name) : Component(name)
    {
        LOG_F(INFO, "Load {}", name);
        def("helloWorld", &MyOtherSharedComponent::helloWorld,
                        PointerSentinel(this));
    }
    virtual ~MyOtherSharedComponent()
    {
        LOG_F(INFO, "Unload {}", getName());
    }

    virtual bool initialize() override { return true; }
    virtual bool destroy() override { return true; }

protected:
    void helloWorld(const std::string &params) {
        std::cout << "Hello " << params << std::endl;
    }
};

int main() {
    std::shared_ptr<MySharedComponent> mycomponent =
        std::make_shared<MySharedComponent>("mycomponent");
    mycomponent->dispatch("helloWorld", std::string("aaa"));
    mycomponent->dispatch("calc", 1, 2);
    auto result =
        mycomponent->dispatch("process", std::vector<int>{1, 2, 3, 4, 5});
    std::cout << std::any_cast<int>(result) << std::endl;


    std::shared_ptr<MyOtherSharedComponent> myothercomponent =
        std::make_shared<MyOtherSharedComponent>("myothercomponent");

    mycomponent->addOtherComponent("other", myothercomponent);
    mycomponent->getOtherComponent("other").lock()->dispatch("helloWorld",
                                                     std::string("bbb"));

    try
    {
        mycomponent->addOtherComponent("other", myothercomponent);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    return 0;
}