#include "atom/components/component.hpp"
#include "atom/function/overload.hpp"
#include "atom/tests/test.hpp"
#include "type/pointer.hpp"

namespace atom::components::test {

// Named constants
constexpr int INITIAL_TEST_VAR_VALUE = 42;
constexpr int UPDATED_TEST_VAR_VALUE = 100;
constexpr int COMMAND_RETURN_VALUE = 42;
constexpr int ADD_INT_FIRST_PARAM = 10;
constexpr int ADD_INT_SECOND_PARAM = 20;
constexpr double ADD_DOUBLE_FIRST_PARAM = 10.0;
constexpr double ADD_DOUBLE_SECOND_PARAM = 20.0;
constexpr double ADD_DOUBLE_RESULT = 30.0;

void TestComponentInitialization() {
    Component component("TestComponent");
    expect(component.initialize() == true);
}

void TestComponentDestruction() {
    Component component("TestComponent");
    expect(component.destroy() == true);
}

void TestComponentVariableManagement() {
    Component component("TestComponent");
    component.addVariable<int>("testVar", INITIAL_TEST_VAR_VALUE);
    auto var = component.getVariable<int>("testVar");
    expect(var->get() == INITIAL_TEST_VAR_VALUE);
    component.setValue<int>("testVar", UPDATED_TEST_VAR_VALUE);
    expect(var->get() == UPDATED_TEST_VAR_VALUE);
}

void TestComponentCommandDispatching() {
    Component component("TestComponent");
    component.def("testCommand", []() { return COMMAND_RETURN_VALUE; });
    auto result = component.dispatch("testCommand");
    expect(std::any_cast<int>(result) == COMMAND_RETURN_VALUE);
}

void testComponentTypeInformation() {
    Component component("TestComponent");
    atom::meta::TypeInfo typeInfo = atom::meta::userType<Component>();
    component.setTypeInfo(typeInfo);
    expect(component.getTypeInfo() == typeInfo);
}

void testComponentOtherComponentManagement() {
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

auto addNumber(int firstParam, int secondParam) -> int {
    std::cout << "addInt" << std::endl;
    return firstParam + secondParam;
}

auto addNumber(double firstParam, double secondParam) -> double {
    std::cout << "addDouble" << std::endl;
    return firstParam + secondParam;
}

void TestComponentFunctionRegistration() {
    Component component("TestComponent");
    {
        component.def("testFunction", []() { return COMMAND_RETURN_VALUE; });
        auto result = component.dispatch("testFunction");
        expect(std::any_cast<int>(result) == COMMAND_RETURN_VALUE);
    }
    {
        component.def("testFunction", [](int firstParam, int secondParam) {
            return firstParam + secondParam;
        });
        auto result = component.dispatch("testFunction", ADD_INT_FIRST_PARAM,
                                         ADD_INT_SECOND_PARAM);
        expect(std::any_cast<int>(result) ==
               ADD_INT_FIRST_PARAM + ADD_INT_SECOND_PARAM);
    }
    {
        component.def<int, int>("testFunction", addNumber);
        auto intResult = component.dispatch("testFunction", ADD_INT_FIRST_PARAM,
                                            ADD_INT_SECOND_PARAM);
        expect(std::any_cast<int>(intResult) ==
               ADD_INT_FIRST_PARAM + ADD_INT_SECOND_PARAM);
        component.def<double, double>("testFunction", addNumber);
        auto doubleResult = component.dispatch(
            "testFunction", ADD_DOUBLE_FIRST_PARAM, ADD_DOUBLE_SECOND_PARAM);
        expect(std::any_cast<double>(doubleResult) ==
               ADD_DOUBLE_FIRST_PARAM + ADD_DOUBLE_SECOND_PARAM);
    }
    {
        component.def("testFunction",
                      atom::meta::overload_cast<int, int>(addNumber));
        auto intResult = component.dispatch("testFunction", ADD_INT_FIRST_PARAM,
                                            ADD_INT_SECOND_PARAM);
        expect(std::any_cast<int>(intResult) ==
               ADD_INT_FIRST_PARAM + ADD_INT_SECOND_PARAM);
        component.def<double, double>(
            "testFunction",
            atom::meta::overload_cast<double, double>(addNumber));
        auto doubleResult = component.dispatch(
            "testFunction", ADD_DOUBLE_FIRST_PARAM, ADD_DOUBLE_SECOND_PARAM);
        expect(std::any_cast<double>(doubleResult) ==
               ADD_DOUBLE_FIRST_PARAM + ADD_DOUBLE_SECOND_PARAM);
    }
}

void TestComponentClassFunctionRegistrationInstance() {
    struct TestClass {
        auto add(int firstParam, int secondParam) const -> int {
            return firstParam + secondParam;
        }
    };

    Component component("TestComponent");
    {
        auto *testClass = new TestClass();
        component.def("testFunction", &TestClass::add, testClass);
        auto result = component.dispatch("testFunction", ADD_INT_FIRST_PARAM,
                                         ADD_INT_SECOND_PARAM);
        expect(std::any_cast<int>(result) ==
               ADD_INT_FIRST_PARAM + ADD_INT_SECOND_PARAM);
        delete testClass;
    }
    {
        auto testClass = std::make_shared<TestClass>();
        component.def("testFunction", &TestClass::add, testClass);
        auto result = component.dispatch("testFunction", ADD_INT_FIRST_PARAM,
                                         ADD_INT_SECOND_PARAM);
        expect(std::any_cast<int>(result) ==
               ADD_INT_FIRST_PARAM + ADD_INT_SECOND_PARAM);
    }
    {
        auto testClass = std::make_unique<TestClass>();
        component.def("testFunction", &TestClass::add, testClass);
        auto result = component.dispatch("testFunction", ADD_INT_FIRST_PARAM,
                                         ADD_INT_SECOND_PARAM);
        expect(std::any_cast<int>(result) ==
               ADD_INT_FIRST_PARAM + ADD_INT_SECOND_PARAM);
    }
    {
        auto testClass = PointerSentinel<TestClass>();
        component.def("testFunction", &TestClass::add, testClass);
        auto result = component.dispatch("testFunction", ADD_INT_FIRST_PARAM,
                                         ADD_INT_SECOND_PARAM);
        expect(std::any_cast<int>(result) ==
               ADD_INT_FIRST_PARAM + ADD_INT_SECOND_PARAM);
    }
}

void registerTests() {
    using namespace atom::test;
    registerTest("Component Initialization", TestComponentInitialization);
    registerTest("Component Destruction", TestComponentDestruction);
    registerTest("Component Variable Management",
                 TestComponentVariableManagement);
    registerTest("Component Command Dispatching",
                 TestComponentCommandDispatching);
    registerTest("Component Type Information", testComponentTypeInformation);
    registerTest("Component Other Component Management",
                 testComponentOtherComponentManagement);
}

}  // namespace atom::components::test