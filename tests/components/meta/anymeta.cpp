#include "atom/function/anymeta.hpp"
#include <gtest/gtest.h>

class MyClass {
public:
    int value;

    MyClass(int v) : value(v) {}

    int get_value() const { return value; }

    void set_value(int v) { value = v; }

    void print() const { std::cout << "MyClass value: " << value << std::endl; }
};

TEST(TypeMetadataTest, ConstructorTest) {
    using namespace atom::meta;

    TypeMetadata metadata;

    // 添加构造函数
    metadata.add_constructor([](std::vector<BoxedValue> args) -> BoxedValue {
        int arg = std::any_cast<int>(args[0].get());
        return BoxedValue(std::make_shared<MyClass>(arg));
    });

    auto constructor = metadata.get_constructor();
    ASSERT_TRUE(constructor.has_value());

    // 使用构造函数创建一个对象
    auto instance = constructor.value()({BoxedValue(42)});
    auto myClassInstance =
        std::any_cast<std::shared_ptr<MyClass>>(instance.get());

    ASSERT_EQ(myClassInstance->get_value(), 42);
}

TEST(TypeMetadataTest, MethodTest) {
    using namespace atom::meta;

    TypeMetadata metadata;

    metadata.add_method(
        "print", [](std::vector<BoxedValue> args) -> BoxedValue {
            auto obj = std::any_cast<std::shared_ptr<MyClass>>(args[0].get());
            obj->print();
            return BoxedValue();
        });

    auto method = metadata.get_method("print");
    ASSERT_TRUE(method.has_value());

    auto myClassInstance = std::make_shared<MyClass>(10);
    method.value()({BoxedValue(myClassInstance)});
}

TEST(TypeMetadataTest, PropertyTest) {
    using namespace atom::meta;

    TypeMetadata metadata;

    metadata.add_property(
        "value",
        [](const BoxedValue& obj) -> BoxedValue {
            return BoxedValue(std::any_cast<std::shared_ptr<MyClass>>(obj.get())
                                  ->get_value());
        },
        [](BoxedValue& obj, const BoxedValue& value) {
            std::any_cast<std::shared_ptr<MyClass>>(obj.get())->set_value(
                std::any_cast<int>(value.get()));
        });

    auto property = metadata.get_property("value");
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
    metadata.add_method(
        "print", [](std::vector<BoxedValue> args) -> BoxedValue {
            auto obj = std::any_cast<std::shared_ptr<MyClass>>(args[0].get());
            obj->print();
            return BoxedValue();
        });

    registry.register_type("MyClass", metadata);

    auto retrievedMetadata = registry.get_metadata("MyClass");
    ASSERT_TRUE(retrievedMetadata.has_value());

    auto method = retrievedMetadata.value().get_method("print");
    ASSERT_TRUE(method.has_value());

    auto myClassInstance = std::make_shared<MyClass>(10);
    method.value()({BoxedValue(myClassInstance)});
}
