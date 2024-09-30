// src/atom/components/test_component.hpp

#ifndef ATOM_COMPONENTS_TEST_COMPONENT_HPP
#define ATOM_COMPONENTS_TEST_COMPONENT_HPP

#include "../atom_test.hpp"
#include "atom/components/component.hpp"
#include "atom/function/overload.hpp"
#include "type/pointer.hpp"

namespace atom::components::test {

void test_component_initialization() {
    Component component("TestComponent");
    expect(component.initialize() == true);
}

void test_component_destruction() {
    Component component("TestComponent");
    expect(component.destroy() == true);
}

void test_component_variable_management() {
    Component component("TestComponent");
    component.addVariable<int>("testVar", 42);
    auto var = component.getVariable<int>("testVar");
    expect(var->get() == 42);
    component.setValue<int>("testVar", 100);
    expect(var->get() == 100);
}

void test_component_command_dispatching() {
    Component component("TestComponent");
    component.def("testCommand", []() { return 42; });
    auto result = component.dispatch("testCommand");
    expect(std::any_cast<int>(result) == 42);
}

void test_component_type_information() {
    Component component("TestComponent");
    atom::meta::TypeInfo typeInfo = atom::meta::userType<Component>();
    component.setTypeInfo(typeInfo);
    expect(component.getTypeInfo() == typeInfo);
}

void test_component_other_component_management() {
    Component component("TestComponent");
    auto otherComponent = std::make_shared<Component>("OtherComponent");
    component.addOtherComponent("OtherComponent", otherComponent);
    auto retrievedComponent =
        component.getOtherComponent("OtherComponent").lock();
    expect(retrievedComponent != nullptr);
    expect(retrievedComponent->getName() == "OtherComponent");
    component.removeOtherComponent("OtherComponent");
    expect(component.getOtherComponent("OtherComponent").expired() == true);
}

int addNumber(int a, int b) {
    std::cout << "addInt" << std::endl;
    return a + b;
}

double addNumber(double a, double b) {
    std::cout << "addDouble" << std::endl;
    return a + b;
}

void test_component_function_registration() {
    Component component("TestComponent");
    {
        component.def("testFunction", []() { return 42; });
        auto result = component.dispatch("testFunction");
        expect(std::any_cast<int>(result) == 42);
    }
    {
        component.def("testFunction", [](int a, int b) { return a + b; });
        auto result = component.dispatch("testFunction", 10, 20);
        expect(std::any_cast<int>(result) == 30);
    }
    {
        component.def<int, int>("testFunction", addNumber);
        auto intResult = component.dispatch("testFunction", 10, 20);
        expect(std::any_cast<int>(intResult) == 30);
        component.def<double, double>("testFunction", addNumber);
        auto doubleResult = component.dispatch("testFunction", 10.0, 20.0);
        expect(std::any_cast<double>(doubleResult) == 30.0);
    }
    {
        component.def("testFunction",
                      atom::meta::overload_cast<int, int>(addNumber));
        auto intResult = component.dispatch("testFunction", 10, 20);
        expect(std::any_cast<int>(intResult) == 30);
        component.def<double, double>(
            "testFunction",
            atom::meta::overload_cast<double, double>(addNumber));
        auto doubleResult = component.dispatch("testFunction", 10.0, 20.0);
        expect(std::any_cast<double>(doubleResult) == 30.0);
    }
}

void test_component_class_function_registration_instance() {
    struct TestClass {
        int add(int a, int b) { return a + b; }
    };

    Component component("TestComponent");
    {
        auto *testClass = new TestClass();
        component.def("testFunction", &TestClass::add, testClass);
        auto result = component.dispatch("testFunction", 10, 20);
        expect(std::any_cast<int>(result) == 30);
        delete testClass;
    }
    {
        auto testClass = std::make_shared<TestClass>();
        component.def("testFunction", &TestClass::add, testClass);
        auto result = component.dispatch("testFunction", 10, 20);
        expect(std::any_cast<int>(result) == 30);
    }
    {
        auto testClass = std::make_unique<TestClass>();
        component.def("testFunction", &TestClass::add, testClass);
        auto result = component.dispatch("testFunction", 10, 20);
        expect(std::any_cast<int>(result) == 30);
    }
    {
        auto testClass = PointerSentinel<TestClass>();
        component.def("testFunction", &TestClass::add, testClass);
        auto result = component.dispatch("testFunction", 10, 20);
        expect(std::any_cast<int>(result) == 30);
    }
}

void register_tests() {
    using namespace atom::test;
    registerTest("Component Initialization", test_component_initialization);
    registerTest("Component Destruction", test_component_destruction);
    registerTest("Component Variable Management",
                 test_component_variable_management);
    registerTest("Component Command Dispatching",
                 test_component_command_dispatching);
    registerTest("Component Type Information", test_component_type_information);
    registerTest("Component Other Component Management",
                 test_component_other_component_management);
}

}  // namespace atom::components::test

#endif  // ATOM_COMPONENTS_TEST_COMPONENT_HPP
