#include "atom/components/component.hpp"
#include <gtest/gtest.h>

using namespace std::literals;

class ComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        component = std::make_shared<Component>("TestComponent");
    }

    void TearDown() override { component.reset(); }

    std::shared_ptr<Component> component;
};

// 基本功能测试
TEST_F(ComponentTest, Initialize) { EXPECT_TRUE(component->initialize()); }

TEST_F(ComponentTest, GetName) {
    EXPECT_EQ(component->getName(), "TestComponent");
}

TEST_F(ComponentTest, GetTypeInfo) {
    component->setTypeInfo(atom::meta::userType<ComponentTest>());
    EXPECT_EQ(component->getTypeInfo(), atom::meta::userType<ComponentTest>());
}

// 变量操作测试
TEST_F(ComponentTest, AddVariables) {
    component->addVariable<int>("intVar", 42, "An integer variable");
    component->addVariable<float>("floatVar", 3.14f, "A float variable");
    component->addVariable<bool>("boolVar", true, "A boolean variable");
    component->addVariable<std::string>("strVar", "Hello", "A string variable");

    EXPECT_EQ(component->getVariable<int>("intVar")->get(), 42);
    EXPECT_FLOAT_EQ(component->getVariable<float>("floatVar")->get(), 3.14f);
    EXPECT_EQ(component->getVariable<bool>("boolVar")->get(), true);
    EXPECT_EQ(component->getVariable<std::string>("strVar")->get(), "Hello");
}

TEST_F(ComponentTest, SetVariableValues) {
    component->addVariable<int>("intVar", 42);
    component->setValue("intVar", 84);
    EXPECT_EQ(component->getVariable<int>("intVar")->get(), 84);
}

// 函数定义测试
TEST_F(ComponentTest, DefineFunctions) {
    int counter = 0;
    component->def("incrementCounter", [&counter]() { ++counter; });
    component->dispatch("incrementCounter", {});
    EXPECT_EQ(counter, 1);
}

TEST_F(ComponentTest, DefineFunctionsWithParameters) {
    component->def("add", [](int a, int b) { return a + b; });
    EXPECT_EQ(std::any_cast<int>(component->dispatch("add", {1, 2})), 3);
}

TEST_F(ComponentTest, DefineFunctionsWithAnyVectorParameters) {
    component->def("add", [](int a, int b, int c) { return a + b + c; });
    std::vector<std::any> args = {1, 2, 3};
    EXPECT_EQ(std::any_cast<int>(component->dispatch("add", args)), 6);
}

TEST_F(ComponentTest, DefineFunctionsWithConstRefStringParameters) {
    component->def("concat",
                   [](std::string a, std::string b) { return a + b; });
    EXPECT_EQ(std::any_cast<std::string>(
                  component->dispatch("concat", "Hello"s, "World"s)),
              "HelloWorld"s);

    component->def("cconcat", [](const std::string a, const std::string b) {
        return a + b;
    });
    EXPECT_EQ(std::any_cast<std::string>(
                  component->dispatch("cconcat", "Hello"s, "World"s)),
              "HelloWorld"s);

    component->def("crconcat", [](const std::string& a, const std::string& b) {
        return a + b;
    });
    EXPECT_EQ(std::any_cast<std::string>(
                  component->dispatch("crconcat", "Hello"s, "World"s)),
              "HelloWorld"s);
}

TEST_F(ComponentTest, DefineMemberFunctions) {
    class TestClass {
    public:
        int testVar = 0;

        int var_getter() const { return testVar; }

        void var_setter(int value) { testVar = value; }
    };

    auto testInstance = std::make_shared<TestClass>();

    component->def("var_getter", &TestClass::var_getter, testInstance);
    component->def("var_setter", &TestClass::var_setter, testInstance);

    EXPECT_EQ(std::any_cast<int>(component->dispatch("var_getter", {})), 0);
    component->dispatch("var_setter", {42});
    EXPECT_EQ(std::any_cast<int>(component->dispatch("var_getter", {})), 42);
}

TEST_F(ComponentTest, DefineMemberFunctionsWithoutInstance) {
    class TestClass {
    public:
        int testVar = 0;

        int var_getter() const { return testVar; }

        void var_setter(int value) { testVar = value; }
    };

    TestClass testInstance;

    component->def("var_getter_without_instance", &TestClass::var_getter);
    component->def("var_setter_without_instance", &TestClass::var_setter);
    EXPECT_TRUE(component->has("var_getter_without_instance"));
    EXPECT_TRUE(component->has("var_setter_without_instance"));
    EXPECT_EQ(std::any_cast<int>(component->dispatch(
                  "var_getter_without_instance", {&testInstance})),
              0);
    component->dispatch("var_setter_without_instance", {&testInstance, 42});
    EXPECT_EQ(std::any_cast<int>(component->dispatch(
                  "var_getter_without_instance", {&testInstance})),
              42);
}

// 构造函数测试
TEST_F(ComponentTest, DefineConstructors) {
    class MyClass {
    public:
        MyClass(int a, std::string b) : testVar(a), testStr(b) {}
        MyClass() : testVar(0), testStr("default") {}

        int testVar;
        std::string testStr;
    };

    component->defConstructor<MyClass, int, std::string>(
        "create_my_class", "MyGroup", "Create MyClass");
    component->defDefaultConstructor<MyClass>(
        "create_default_my_class", "MyGroup", "Create default MyClass");

    auto class_with_args =
        component->dispatch("create_my_class", {1, std::string("args")});
    auto default_class = component->dispatch("create_default_my_class", {});

    EXPECT_EQ(std::any_cast<std::shared_ptr<MyClass>>(class_with_args)->testVar,
              1);
    EXPECT_EQ(std::any_cast<std::shared_ptr<MyClass>>(class_with_args)->testStr,
              "args");
    EXPECT_EQ(std::any_cast<std::shared_ptr<MyClass>>(default_class)->testVar,
              0);
    EXPECT_EQ(std::any_cast<std::shared_ptr<MyClass>>(default_class)->testStr,
              "default");
}

// 类型定义测试
TEST_F(ComponentTest, DefineTypes) {
    class TestClass {};
    component->defType<TestClass>("TestClass",
                                  atom::meta::userType<TestClass>());
    EXPECT_TRUE(component->hasType("TestClass"));
}

TEST_F(ComponentTest, DefineClass) {
    class TestClass {
    public:
        int testVar = 0;

        TestClass() = default;

        explicit TestClass(int value) : testVar(value) {}

        auto varGetter() const -> int { return testVar; }

        void varSetter(int value) { testVar = value; }
    };

    component->doc("This is a test class");
    component->defType<TestClass>("TestClass",
                                  atom::meta::userType<TestClass>(), "MyGroup",
                                  "Test class");
    component->defConstructor<TestClass, int>("create_test_class", "MyGroup",
                                              "Create TestClass");
    component->defDefaultConstructor<TestClass>(
        "create_default_test_class", "MyGroup", "Create default TestClass");
    component->def("var_getter", &TestClass::varGetter, "MyGroup",
                   "Get testVar");
    component->def("var_setter", &TestClass::varSetter, "MyGroup",
                   "Set testVar");
}

// 错误处理测试
TEST_F(ComponentTest, ErrorHandling) {
    // 尝试获取不存在的变量
    EXPECT_FALSE(component->hasVariable("nonExistentVar"));

    // 尝试调用不存在的函数
    EXPECT_THROW(component->dispatch("nonExistentFunction", {}),
                 atom::error::InvalidArgument);
}

// 性能测试（示例）
TEST_F(ComponentTest, Performance) {
    // 添加大量变量
    for (int i = 0; i < 1000; ++i) {
        component->addVariable<int>(std::to_string(i), i,
                                    "Integer variable " + std::to_string(i));
    }

    // 测试获取变量的性能
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; ++i) {
        component->getVariable<int>(std::to_string(i));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    // 这里可以添加断言来检查性能是否在可接受范围内
    std::cout << "Time to get 1000 variables: " << duration.count()
              << " microseconds" << std::endl;
}

// 边界条件测试
TEST_F(ComponentTest, BoundaryConditions) {
    // 测试整数变量的边界
    component->addVariable<int>("minInt", std::numeric_limits<int>::min());
    component->addVariable<int>("maxInt", std::numeric_limits<int>::max());

    EXPECT_EQ(component->getVariable<int>("minInt")->get(),
              std::numeric_limits<int>::min());
    EXPECT_EQ(component->getVariable<int>("maxInt")->get(),
              std::numeric_limits<int>::max());
}

#include <chrono>
#include <thread>

TEST_F(ComponentTest, ThreadSafety) {
    // 假设 component 是线程安全的
    component->addVariable<int>("sharedVar", 0, "A shared variable");

    std::thread thread1([&]() {
        for (int i = 0; i < 1000; ++i) {
            component->setValue("sharedVar", i);
        }
    });

    std::thread thread2([&]() {
        for (int i = 1000; i > 0; --i) {
            component->setValue("sharedVar", i);
        }
    });

    thread1.join();
    thread2.join();

    // 检查共享变量的最终值是否在预期范围内
    EXPECT_TRUE(component->getVariable<int>("sharedVar")->get() >= 0 &&
                component->getVariable<int>("sharedVar")->get() <= 1000);
}

// 组件生命周期测试
TEST_F(ComponentTest, Lifecycle) {
    EXPECT_TRUE(component->destroy());
    // 组件销毁后，操作应该失败
    EXPECT_FALSE(component->getVariable<int>("intVar"));
    EXPECT_THROW(component->dispatch("incrementCounter", {}),
                 atom::error::InvalidArgument);
}
