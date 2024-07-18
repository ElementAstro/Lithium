// test_var_manager.cpp
#include "atom/components/var.hpp"
#include <gtest/gtest.h>
#include <string>
#include "error/exception.hpp"

struct TestClass {
    int intValue;
    std::string stringValue;
};

TEST(VariableManagerTest, AddAndGetVariable) {
    VariableManager vm;

    vm.addVariable("intVar", 42, "An integer variable", "intVarAlias",
                   "group1");
    vm.addVariable("stringVar", std::string("Hello"), "A string variable");

    auto intVar = vm.getVariable<int>("intVar");
    ASSERT_NE(intVar, nullptr);
    EXPECT_EQ(intVar->get(), 42);

    auto stringVar = vm.getVariable<std::string>("stringVar");
    ASSERT_NE(stringVar, nullptr);
    EXPECT_EQ(stringVar->get(), "Hello");

    EXPECT_TRUE(vm.has("intVar"));
    EXPECT_FALSE(vm.has("nonExistentVar"));
}

TEST(VariableManagerTest, SetAndGetValue) {
    VariableManager vm;

    vm.addVariable("intVar", 42);
    vm.setValue("intVar", 84);
    auto intVar = vm.getVariable<int>("intVar");
    ASSERT_NE(intVar, nullptr);
    EXPECT_EQ(intVar->get(), 84);

    vm.addVariable("stringVar", std::string("Hello"));
    vm.setValue("stringVar", std::string("World"));
    auto stringVar = vm.getVariable<std::string>("stringVar");
    ASSERT_NE(stringVar, nullptr);
    EXPECT_EQ(stringVar->get(), "World");
}

TEST(VariableManagerTest, SetRangeAndValueOutOfRange) {
    VariableManager vm;

    vm.addVariable("intVar", 42);
    vm.setRange("intVar", 0, 100);

    vm.setValue("intVar", 50);
    auto intVar = vm.getVariable<int>("intVar");
    EXPECT_EQ(intVar->get(), 50);

    EXPECT_THROW(vm.setValue("intVar", 150), atom::error::OutOfRange);
    EXPECT_EQ(intVar->get(), 50);
}

TEST(VariableManagerTest, SetStringOptions) {
    VariableManager vm;

    vm.addVariable("stringVar", std::string("Option1"));
    vm.setStringOptions("stringVar", {"Option1", "Option2", "Option3"});

    vm.setValue("stringVar", std::string("Option2"));
    auto stringVar = vm.getVariable<std::string>("stringVar");
    ASSERT_NE(stringVar, nullptr);
    EXPECT_EQ(stringVar->get(), "Option2");

    EXPECT_THROW(vm.setValue("stringVar", std::string("InvalidOption")),
                 atom::error::InvalidArgument);
    EXPECT_EQ(stringVar->get(), "Option2");
}

TEST(VariableManagerTest, ClassMemberVariable) {
    struct TestClass {
        int intValue;
        std::string stringValue;
    };

    TestClass obj{42, "Hello"};
    VariableManager vm;

    vm.addVariable("intMember", &TestClass::intValue, obj,
                   "Integer member variable");
    vm.addVariable("stringMember", &TestClass::stringValue, obj,
                   "String member variable");

    auto intMember = vm.getVariable<int>("intMember");
    ASSERT_NE(intMember, nullptr);
    EXPECT_EQ(intMember->get(), 42);

    auto stringMember = vm.getVariable<std::string>("stringMember");
    ASSERT_NE(stringMember, nullptr);
    EXPECT_EQ(stringMember->get(), "Hello");

    vm.setValue("intMember", 84);
    vm.setValue("stringMember", std::string("World"));

    EXPECT_EQ(obj.intValue, 84);
    EXPECT_EQ(obj.stringValue, "World");
}

TEST(VariableManagerTest, GetDescriptionAliasGroup) {
    VariableManager vm;

    vm.addVariable("var1", 42, "Description for var1", "alias1", "group1");
    vm.addVariable("var2", std::string("Hello"), "Description for var2",
                   "alias2", "group2");

    EXPECT_EQ(vm.getDescription("var1"), "Description for var1");
    EXPECT_EQ(vm.getDescription("alias1"), "Description for var1");

    EXPECT_EQ(vm.getAlias("var1"), "alias1");
    EXPECT_EQ(vm.getAlias("alias1"), "var1");

    EXPECT_EQ(vm.getGroup("var1"), "group1");
    EXPECT_EQ(vm.getGroup("alias1"), "group1");

    EXPECT_EQ(vm.getDescription("var2"), "Description for var2");
    EXPECT_EQ(vm.getDescription("alias2"), "Description for var2");

    EXPECT_EQ(vm.getAlias("var2"), "alias2");
    EXPECT_EQ(vm.getAlias("alias2"), "var2");

    EXPECT_EQ(vm.getGroup("var2"), "group2");
    EXPECT_EQ(vm.getGroup("alias2"), "group2");
}
