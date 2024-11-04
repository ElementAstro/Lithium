#include "atom/components/component.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <string>

// Test fixture for Component
class ComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        component = std::make_shared<Component>("TestComponent");
    }

    std::shared_ptr<Component> component;
};

// Test constructor
TEST_F(ComponentTest, Constructor) {
    EXPECT_EQ(component->getName(), "TestComponent");
}

// Test getInstance
TEST_F(ComponentTest, GetInstance) {
    auto weakInstance = component->getInstance();
    EXPECT_FALSE(weakInstance.expired());
}

// Test getSharedInstance
TEST_F(ComponentTest, GetSharedInstance) {
    auto sharedInstance = component->getSharedInstance();
    EXPECT_EQ(sharedInstance, component);
}

// Test initialize (default implementation)
TEST_F(ComponentTest, Initialize) { EXPECT_FALSE(component->initialize()); }

// Test destroy (default implementation)
TEST_F(ComponentTest, Destroy) { EXPECT_FALSE(component->destroy()); }

// Test getName
TEST_F(ComponentTest, GetName) {
    EXPECT_EQ(component->getName(), "TestComponent");
}

// Test getTypeInfo and setTypeInfo
TEST_F(ComponentTest, GetSetTypeInfo) {
    atom::meta::TypeInfo typeInfo = atom::meta::userType<int>();
    component->setTypeInfo(typeInfo);
    EXPECT_EQ(component->getTypeInfo(), typeInfo);
}

// Test addVariable, getVariable, and hasVariable
TEST_F(ComponentTest, AddGetHasVariable) {
    component->addVariable<int>("var1", 10, "Test variable");
    auto var = component->getVariable<int>("var1");
    EXPECT_EQ(var->get(), 10);
    EXPECT_TRUE(component->hasVariable("var1"));
}

// Test setRange
TEST_F(ComponentTest, SetRange) {
    component->addVariable<int>("var2", 5);
    component->setRange<int>("var2", 1, 10);
    // Assuming VariableManager has a method to get range (not shown in the
    // provided code)
}

// Test setStringOptions
TEST_F(ComponentTest, SetStringOptions) {
    component->addVariable<std::string>("var3", "option1");
    std::vector<std::string> options = {"option1", "option2", "option3"};
    component->setStringOptions("var3", options);
    // Assuming VariableManager has a method to get options (not shown in the
    // provided code)
}

// Test setValue
TEST_F(ComponentTest, SetValue) {
    component->addVariable<int>("var4", 20);
    component->setValue<int>("var4", 30);
    auto var = component->getVariable<int>("var4");
    EXPECT_EQ(var->get(), 30);
}

// Test getVariableNames
TEST_F(ComponentTest, GetVariableNames) {
    component->addVariable<int>("var5", 50);
    auto names = component->getVariableNames();
    EXPECT_EQ(names.size(), 1);
    EXPECT_EQ(names[0], "var5");
}

// Test getVariableDescription
TEST_F(ComponentTest, GetVariableDescription) {
    component->addVariable<int>("var6", 60, "Description for var6");
    EXPECT_EQ(component->getVariableDescription("var6"),
              "Description for var6");
}

// Test getVariableAlias
TEST_F(ComponentTest, GetVariableAlias) {
    component->addVariable<int>("var7", 70, "", "alias_var7");
    EXPECT_EQ(component->getVariableAlias("var7"), "alias_var7");
}

// Test getVariableGroup
TEST_F(ComponentTest, GetVariableGroup) {
    component->addVariable<int>("var8", 80, "", "", "group_var8");
    EXPECT_EQ(component->getVariableGroup("var8"), "group_var8");
}

// Test doc and getDoc
TEST_F(ComponentTest, DocAndGetDoc) {
    component->doc("Component documentation");
    EXPECT_EQ(component->getDoc(), "Component documentation");
}

// Test dispatch
TEST_F(ComponentTest, Dispatch) {
    component->def("testCommand", []() { return 42; });
    auto result = std::any_cast<int>(component->dispatch("testCommand"));
    EXPECT_EQ(result, 42);
}

// Test has
TEST_F(ComponentTest, Has) {
    component->def("testCommand2", []() { return 42; });
    EXPECT_TRUE(component->has("testCommand2"));
}

// Test getCommandsInGroup
TEST_F(ComponentTest, GetCommandsInGroup) {
    component->def("testCommand3", []() { return 42; }, "group1");
    auto commands = component->getCommandsInGroup("group1");
    EXPECT_EQ(commands.size(), 1);
    EXPECT_EQ(commands[0], "testCommand3");
}

// Test getCommandDescription
TEST_F(ComponentTest, GetCommandDescription) {
    component->def(
        "testCommand4", []() { return 42; }, "",
        "Description for testCommand4");
    EXPECT_EQ(component->getCommandDescription("testCommand4"),
              "Description for testCommand4");
}

// Test getCommandArgAndReturnType
TEST_F(ComponentTest, GetCommandArgAndReturnType) {
    component->def("testCommand5", [](int a) { return a; });
    auto [args, ret] = component->getCommandArgAndReturnType("testCommand5");
    EXPECT_EQ(args.size(), 1);
    EXPECT_EQ(ret, "int");
}

// Test getAllCommands
TEST_F(ComponentTest, GetAllCommands) {
    component->def("testCommand6", []() { return 42; });
    auto commands = component->getAllCommands();
    EXPECT_EQ(commands.size(), 1);
    EXPECT_EQ(commands[0], "testCommand6");
}

// Test getRegisteredTypes
TEST_F(ComponentTest, GetRegisteredTypes) {
    component->defType<int>("intType");
    auto types = component->getRegisteredTypes();
    EXPECT_EQ(types.size(), 1);
    EXPECT_EQ(types[0], "intType");
}

// Test getNeededComponents
TEST_F(ComponentTest, GetNeededComponents) {
    auto neededComponents = Component::getNeededComponents();
    EXPECT_TRUE(neededComponents.empty());
}

// Test addOtherComponent, getOtherComponent, and removeOtherComponent
TEST_F(ComponentTest, AddGetRemoveOtherComponent) {
    auto otherComponent = std::make_shared<Component>("OtherComponent");
    component->addOtherComponent("OtherComponent", otherComponent);
    auto retrievedComponent =
        component->getOtherComponent("OtherComponent").lock();
    EXPECT_EQ(retrievedComponent, otherComponent);
    component->removeOtherComponent("OtherComponent");
    EXPECT_TRUE(component->getOtherComponent("OtherComponent").expired());
}

// Test clearOtherComponents
TEST_F(ComponentTest, ClearOtherComponents) {
    auto otherComponent1 = std::make_shared<Component>("OtherComponent1");
    auto otherComponent2 = std::make_shared<Component>("OtherComponent2");
    component->addOtherComponent("OtherComponent1", otherComponent1);
    component->addOtherComponent("OtherComponent2", otherComponent2);
    component->clearOtherComponents();
    EXPECT_TRUE(component->getOtherComponent("OtherComponent1").expired());
    EXPECT_TRUE(component->getOtherComponent("OtherComponent2").expired());
}

// Test runCommand
TEST_F(ComponentTest, RunCommand) {
    component->def("testCommand7", [](int a, int b) { return a + b; });
    std::vector<std::any> args = {1, 2};
    auto result =
        std::any_cast<int>(component->runCommand("testCommand7", args));
    EXPECT_EQ(result, 3);
}