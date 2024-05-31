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
    bool result = component->initialize();
    EXPECT_TRUE(result);
}

TEST_F(ComponentTest, GetName) {
    std::string name = component->getName();
    EXPECT_EQ(name, "TestComponent");
}

TEST_F(ComponentTest, GetTypeInfo) {
    atom::meta::Type_Info typeInfo = component->getTypeInfo();
    EXPECT_EQ(typeInfo, atom::meta::user_type<Component>());
}

TEST_F(ComponentTest, SetTypeInfo) {
    component->setTypeInfo(atom::meta::user_type<ComponentTest>());
    EXPECT_EQ(component->getTypeInfo(), atom::meta::user_type<ComponentTest>());
    std::cout << component->getTypeInfo().name() << std::endl;
    std::cout << component->getTypeInfo().bare_name() << std::endl;
}

TEST_F(ComponentTest, AddVariable) {
    std::string name = "testVariable";
    int initialValue = 42;
    std::string description = "Test variable";
    std::string alias = "tv";
    std::string group = "TestGroup";

    component->addVariable<int>(name, initialValue, description, alias, group);

    auto variable = component->getVariable<int>(name);
    EXPECT_TRUE(variable);
    EXPECT_EQ(variable->get(), initialValue);

    EXPECT_EQ(component->getVariableDescription(name), description);
    EXPECT_EQ(component->getVariableAlias(name), alias);
    EXPECT_EQ(component->getVariableGroup(name), group);
}

TEST_F(ComponentTest, SetVariableValue) {
    std::string name = "Variable";
    int initialValue = 42;
    int newValue = 84;

    component->addVariable<int>(name, initialValue);

    component->setValue(name, newValue);

    auto variable = component->getVariable<int>(name);
    EXPECT_EQ(variable->get(), newValue);
    EXPECT_EQ(variable->getTypeName(),
              atom::meta::DemangleHelper::DemangleType<int>());
}

TEST_F(ComponentTest, DefFunction) {
    auto counter = 0;
    std::string functionName = "incrementCounter";
    component->def(functionName, [this, &counter]() mutable { ++counter; });

    component->dispatch(functionName, {});

    // Assert
    EXPECT_EQ(counter, 1);
}

TEST_F(ComponentTest, DefVariableMember) {
    class TestClass {
    public:
        int testVar = 0;

        int var_getter() const {
            std::cout << "getter called" << std::endl;
            return testVar;
        }
        void var_setter(int value) {
            std::cout << "setter called" << std::endl;
            std::cout << "value: " << value << std::endl;
            testVar = value;
        }
    };

    std::shared_ptr<TestClass> testInstance = std::make_shared<TestClass>();

    component->def("var_getter", &TestClass::var_getter, testInstance);
    component->def("var_setter", &TestClass::var_setter, testInstance);
    EXPECT_TRUE(component->has("var_getter"));
    EXPECT_TRUE(component->has("var_setter"));
    EXPECT_EQ(std::any_cast<int>(component->dispatch("var_getter", {})), 0);
    component->dispatch("var_setter", {42});
    int value = std::any_cast<int>(component->dispatch("var_getter", {}));
    std::cout << "value: " << value << std::endl;
    EXPECT_EQ(value, 42);

    component->def("testVar", &TestClass::testVar, testInstance);
    EXPECT_TRUE(component->has("get_testVar"));

    component->def("getter", &TestClass::var_getter, &TestClass::var_setter,
                   testInstance);

    component->dispatch("var_setter", {114514});
    value = std::any_cast<int>(component->dispatch("var_getter", {}));
    std::cout << "value: " << value << std::endl;
    EXPECT_EQ(value, 114514);

    component->def_v("test.var", &TestClass::testVar);
    EXPECT_TRUE(component->has("test.var"));
    value = std::any_cast<int>(
        component->dispatch("test.var", {testInstance.get()}));
    std::cout << "value: " << value << std::endl;
    EXPECT_EQ(value, 114514);
}

TEST_F(ComponentTest, DefType) {
    class TestClass {
    public:
        int testVar = 0;
    };
    component->def_type<TestClass>("TestClass",
                                   atom::meta::user_type<TestClass>());
    EXPECT_TRUE(component->has_type("TestClass"));
}

TEST_F(ComponentTest, DefConstructor) {
    class MyClass {
    public:
        MyClass(int a, std::string b) {}
        MyClass() {}
    };
    component->def_constructor<MyClass, int, std::string>(
        "create_my_class", "MyGroup", "Create MyClass");
    component->def_default_constructor<MyClass>(
        "create_default_my_class", "MyGroup", "Create default MyClass");
}

TEST_F(ComponentTest, Destroy) {
    bool result = component->destroy();
    EXPECT_TRUE(result);
}