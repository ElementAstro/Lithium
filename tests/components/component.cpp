#include "atom/components/component.hpp"
#include <gtest/gtest.h>

class ComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        component = std::make_shared<Component>("TestComponent");
    }

    void TearDown() override { component.reset(); }

    std::shared_ptr<Component> component;
};

TEST_F(ComponentTest, Initialize) {
    // Arrange

    // Act
    bool result = component->initialize();

    // Assert
    EXPECT_TRUE(result);
}

TEST_F(ComponentTest, GetName) {
    // Arrange

    // Act
    std::string name = component->getName();

    // Assert
    EXPECT_EQ(name, "TestComponent");
}

TEST_F(ComponentTest, GetTypeInfo) {
    // Arrange

    // Act
    atom::meta::Type_Info typeInfo = component->getTypeInfo();

    // Assert
    // Add assertions for the type info properties
}

TEST_F(ComponentTest, AddVariable) {
    // Arrange
    std::string name = "testVariable";
    int initialValue = 42;
    std::string description = "Test variable";
    std::string alias = "tv";
    std::string group = "TestGroup";

    // Act
    component->addVariable<int>(name, initialValue, description, alias, group);

    auto variable = component->getVariable<int>(name);
    EXPECT_TRUE(variable);
    EXPECT_EQ(variable->get(), initialValue);

    EXPECT_EQ(component->getVariableDescription(name), description);
    EXPECT_EQ(component->getVariableAlias(name), alias);
    EXPECT_EQ(component->getVariableGroup(name), group);
}

TEST_F(ComponentTest, SetVariableValue) {
    // Arrange
    std::string name = "Variable";
    int initialValue = 42;
    int newValue = 84;

    // Add variable
    component->addVariable<int>(name, initialValue);

    // Act
    component->setValue(name, newValue);

    // Assert
    auto variable = component->getVariable<int>(name);
    EXPECT_EQ(variable->get(), newValue);
    EXPECT_EQ(variable->getTypeName(),
              atom::meta::DemangleHelper::DemangleType<int>());
}

TEST_F(ComponentTest, DefFunction) {
    // Arrange
    auto counter = 0;
    std::string functionName = "incrementCounter";
    component->def(functionName, [this, &counter]() mutable { ++counter; });

    // Act
    component->dispatch(functionName, {});

    // Assert
    EXPECT_EQ(counter, 1);
}

TEST_F(ComponentTest, DefVariableMember) {
    // Arrange
    class TestClass {
    public:
        int testVar = 0;
    };

    std::shared_ptr<TestClass> testInstance = std::make_shared<TestClass>();

    // Act
    // component->def("testVar", &TestClass::testVar, testInstance);
    // EXPECT_TRUE(component->has("testVar"));
}

TEST_F(ComponentTest, Destroy) {
    // Arrange

    // Act
    bool result = component->destroy();

    // Assert
    EXPECT_TRUE(result);
}