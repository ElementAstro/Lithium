#include "gtest/gtest.h"

#include "atom/function/anymeta.hpp"  // 包含我们之前定义的 `TypeMetadata` 和 `TypeRegistry`

using namespace atom::meta;

// 测试类
class TestClass {
public:
    int value = 0;

    TestClass() = default;

    explicit TestClass(int v) : value(v) {}

    int getValue() const { return value; }

    void setValue(int v) {
        std::cout << "Setting value to " << v << std::endl;
        value = v;
    }

    void printValue() const { std::cout << "Value: " << value << std::endl; }

    static void staticPrint() { std::cout << "Static print" << std::endl; }
};

// 注册类型信息
class TestClassRegistrar {
public:
    static void registerType() {
        TypeMetadata metadata;

        // 注册构造函数
        metadata.addConstructor(
            "TestClass", [](std::vector<BoxedValue> args) -> BoxedValue {
                if (args.empty()) {
                    return BoxedValue(TestClass{});  // 默认构造函数
                } else if (args.size() == 1) {
                    if (auto value = args[0].tryCast<int>();
                        value.has_value()) {
                        return BoxedValue(TestClass{*value});
                    }
                }
                THROW_INVALID_ARGUMENT("Invalid arguments for constructor");
            });

        // 注册方法
        metadata.addMethod(
            "getValue", [](std::vector<BoxedValue> args) -> BoxedValue {
                auto& obj = args[0];
                auto value = obj.tryCast<TestClass>()->getValue();
                std::cout << "Value: " << value << std::endl;
                return BoxedValue(value);
            });

        metadata.addMethod(
            "setValue", [](std::vector<BoxedValue> args) -> BoxedValue {
                auto& obj = args[0];
                if (auto value = args[1].tryCast<int>(); value.has_value()) {
                    obj.tryCast<TestClass>()->setValue(*value);
                    return BoxedValue{};
                }
                THROW_INVALID_ARGUMENT("Invalid argument for setValue");
            });

        metadata.addMethod("printValue",
                           [](std::vector<BoxedValue> args) -> BoxedValue {
                               args[0].tryCast<TestClass>()->printValue();
                               return BoxedValue{};
                           });

        // 注册属性
        metadata.addProperty(
            "value",
            [](const BoxedValue& obj) -> BoxedValue {
                return BoxedValue(obj.tryCast<TestClass>()->getValue());
            },
            [](BoxedValue& obj, const BoxedValue& value) {
                std::cout << "Setting value to " << value.getTypeInfo().name()
                          << ": " << value.tryCast<int>().value() << std::endl;
                if (auto v = value.tryCast<int>(); v.has_value()) {
                    obj.tryCast<TestClass>()->setValue(*v);
                    std::cout << "Value set to " << *v << std::endl;
                } else {
                    THROW_INVALID_ARGUMENT("Invalid type for value property");
                }
            });

        // 注册事件
        metadata.addEvent("onValueChanged");

        TypeRegistry::instance().registerType("TestClass", std::move(metadata));
    }
};

// 测试套件
class TypeRegistryTest : public ::testing::Test {
protected:
    void SetUp() override { TestClassRegistrar::registerType(); }
};

// 测试：类型注册和构造函数
TEST_F(TypeRegistryTest, TypeRegistrationAndConstructor) {
    auto metadata = TypeRegistry::instance().getMetadata("TestClass");
    ASSERT_TRUE(metadata.has_value());

    // 测试默认构造函数
    BoxedValue obj = createInstance("TestClass", {});
    ASSERT_TRUE(obj.canCast<TestClass>());
    EXPECT_EQ(obj.tryCast<TestClass>()->getValue(), 0);

    // 测试带参数的构造函数
    BoxedValue objWithArg = createInstance("TestClass", {BoxedValue(42)});
    ASSERT_TRUE(objWithArg.canCast<TestClass>());
    EXPECT_EQ(objWithArg.tryCast<TestClass>()->getValue(), 42);

    // 边缘情况：无效参数的构造函数
    EXPECT_THROW(createInstance("TestClass", {BoxedValue("invalid")}),
                 std::exception);
}

// TODO: FIX ME -  测试：方法调用
/*
TEST_F(TypeRegistryTest, MethodCall) {
    BoxedValue obj = createInstance("TestClass", {BoxedValue(10)});
    std::cout << "obj: " << obj.getTypeInfo().name() << std::endl;

    // 调用 getValue 方法
    BoxedValue result = callMethod(obj, "getValue", {});
    ASSERT_TRUE(result.canCast<int>());
    EXPECT_EQ(result.tryCast<int>(), 10);

    // 调用 setValue 方法
    callMethod(obj, "setValue", {BoxedValue(20)});
    result = callMethod(obj, "getValue", {});
    EXPECT_EQ(result.tryCast<int>(), 20);

    // 边缘情况：调用未注册的方法
    EXPECT_THROW(callMethod(obj, "nonexistentMethod", {}), std::exception);
}
*/

// TODO: FIX ME - 测试：属性访问
/*
TEST_F(TypeRegistryTest, PropertyAccess) {
    BoxedValue obj = createInstance("TestClass", {BoxedValue(5)});

    // 获取属性值
    BoxedValue value = getProperty(obj, "value");
    ASSERT_TRUE(value.canCast<int>());
    EXPECT_EQ(value.tryCast<int>(), 5);

    // 设置属性值
    setProperty(obj, "value", BoxedValue(15));
    std::cout << "obj: " << obj.getTypeInfo().name() << std::endl;
    value = getProperty(obj, "value");
    EXPECT_EQ(value.tryCast<int>(), 15);

    // 边缘情况：获取未注册的属性
    EXPECT_THROW(getProperty(obj, "nonexistentProperty"), std::exception);

    // 边缘情况：设置无效类型的属性
    EXPECT_THROW(setProperty(obj, "value", BoxedValue("invalid")),
                 std::exception);
}
*/

// TODO: FIX ME - 测试：事件处理
/*
TEST_F(TypeRegistryTest, EventHandling) {
    BoxedValue obj = createInstance("TestClass", {BoxedValue(5)});

    auto metadata = TypeRegistry::instance().getMetadata("TestClass");
    ASSERT_TRUE(metadata.has_value());

    // 添加事件监听器
    bool eventTriggered = false;
    metadata->addEventListener(
        "onValueChanged", [&](BoxedValue& obj, const std::vector<BoxedValue>&) {
            eventTriggered = true;
        });

    // 触发事件
    fireEvent(obj, "onValueChanged", {});
    EXPECT_TRUE(eventTriggered);

    // 边缘情况：触发不存在的事件
    EXPECT_NO_THROW(fireEvent(obj, "nonexistentEvent", {}));
}
*/

// 测试：构造函数边缘情况
TEST_F(TypeRegistryTest, ConstructorEdgeCases) {
    // 空参数列表的构造函数应该生成默认对象
    BoxedValue defaultObj = createInstance("TestClass", {});
    ASSERT_TRUE(defaultObj.canCast<TestClass>());
    EXPECT_EQ(defaultObj.tryCast<TestClass>()->getValue(), 0);

    // 边缘情况：传递多个无效参数
    EXPECT_THROW(createInstance("TestClass", {BoxedValue(1), BoxedValue(2)}),
                 std::exception);
}

// 测试：多线程访问
TEST_F(TypeRegistryTest, MultithreadedAccess) {
    BoxedValue obj = createInstance("TestClass", {BoxedValue(5)});

    // 在多个线程中读取属性值
    std::vector<std::jthread> threads;
    threads.reserve(10);
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 100; ++j) {
                BoxedValue value = getProperty(obj, "value");
                EXPECT_TRUE(value.canCast<int>());
            }
        });
    }

    // 在多个线程中设置属性值
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 100; ++j) {
                setProperty(obj, "value", BoxedValue(10 + j));
            }
        });
    }

    // 等待所有线程完成
    for (auto& thread : threads) {
        thread.join();
    }

    // 检查最终属性值
    BoxedValue finalValue = getProperty(obj, "value");
    ASSERT_TRUE(finalValue.canCast<int>());
    EXPECT_GE(finalValue.tryCast<int>(),
              5);  // 最终值至少应该是设置过的最大值之一
}
