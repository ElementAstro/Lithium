#include "atom/function/anymeta.hpp"
#include <gtest/gtest.h>

class MyClass {
public:
    int value;

    MyClass(int v) : value(v) {}

    auto getValue() const -> int { return value; }

    void setValue(int v) { value = v; }

    void print() const { std::cout << "MyClass value: " << value << std::endl; }
};

TEST(TypeMetadataTest, ConstructorTest) {
    using namespace atom::meta;

    TypeMetadata metadata;

    // 添加构造函数
    metadata.addConstructor([](std::vector<BoxedValue> args) -> BoxedValue {
        int arg = std::any_cast<int>(args[0].get());
        return BoxedValue(std::make_shared<MyClass>(arg));
    });

    auto constructor = metadata.getConstructor();
    ASSERT_TRUE(constructor.has_value());

    // 使用构造函数创建一个对象
    auto instance = constructor.value()({BoxedValue(42)});
    auto myClassInstance =
        std::any_cast<std::shared_ptr<MyClass>>(instance.get());

    ASSERT_EQ(myClassInstance->getValue(), 42);
}

TEST(TypeMetadataTest, MethodTest) {
    using namespace atom::meta;

    TypeMetadata metadata;

    metadata.addMethod("print", [](std::vector<BoxedValue> args) -> BoxedValue {
        auto obj = std::any_cast<std::shared_ptr<MyClass>>(args[0].get());
        obj->print();
        return {};
    });

    auto method = metadata.getMethod("print");
    ASSERT_TRUE(method.has_value());

    auto myClassInstance = std::make_shared<MyClass>(10);
    method.value()({BoxedValue(myClassInstance)});
}

TEST(TypeMetadataTest, PropertyTest) {
    using namespace atom::meta;

    TypeMetadata metadata;

    metadata.addProperty(
        "value",
        [](const BoxedValue& obj) -> BoxedValue {
            return BoxedValue(std::any_cast<std::shared_ptr<MyClass>>(obj.get())
                                  ->getValue());
        },
        [](BoxedValue& obj, const BoxedValue& value) {
            std::any_cast<std::shared_ptr<MyClass>>(obj.get())->setValue(
                std::any_cast<int>(value.get()));
        });

    auto property = metadata.getProperty("value");
    ASSERT_TRUE(property.has_value());

    auto myClassInstance = std::make_shared<MyClass>(10);
    BoxedValue boxedInstance(myClassInstance);

    int value =
        std::any_cast<int>(property.value().getter(boxedInstance).get());
    ASSERT_EQ(value, 10);

    property.value().setter(boxedInstance, BoxedValue(20));
    value = std::any_cast<int>(property.value().getter(boxedInstance).get());
    ASSERT_EQ(value, 20);
}

TEST(TypeRegistryTest, RegisterAndGetTypeTest) {
    using namespace atom::meta;

    TypeRegistry& registry = TypeRegistry::instance();

    TypeMetadata metadata;
    metadata.addMethod("print", [](std::vector<BoxedValue> args) -> BoxedValue {
        auto obj = std::any_cast<std::shared_ptr<MyClass>>(args[0].get());
        obj->print();
        return {};
    });

    registry.registerType("MyClass", metadata);

    auto retrievedMetadata = registry.getMetadata("MyClass");
    ASSERT_TRUE(retrievedMetadata.has_value());

    auto method = retrievedMetadata.value().getMethod("print");
    ASSERT_TRUE(method.has_value());

    auto myClassInstance = std::make_shared<MyClass>(10);
    method.value()({BoxedValue(myClassInstance)});
}
