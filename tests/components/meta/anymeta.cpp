// test_anymeta.hpp
#include <gtest/gtest.h>
#include "atom/function/anymeta.hpp"
#include <string>

namespace atom::meta::test {

// Test helper class
class TestClass {
public:
    int value{0};
    std::string name;
    
    TestClass() = default;
    TestClass(int v) : value(v) {}
    
    int getValue() const { return value; }
    void setValue(int v) { value = v; }
    std::string getName() const { return name; }
};

class TypeMetadataTest : public ::testing::Test {
protected:
    TypeMetadata metadata;
    
    void SetUp() override {
        // Register a test method
        metadata.addMethod("testMethod", 
            [](std::vector<BoxedValue> args) -> BoxedValue {
                return BoxedValue(42);
            });
            
        // Register a test property
        metadata.addProperty("testProperty",
            [](const BoxedValue& obj) -> BoxedValue {
                return BoxedValue(123);
            },
            [](BoxedValue& obj, const BoxedValue& value) {
                // Setter implementation
            });
            
        // Register a test constructor
        metadata.addConstructor("TestClass",
            [](std::vector<BoxedValue> args) -> BoxedValue {
                return BoxedValue(TestClass{});
            });
    }
};

TEST_F(TypeMetadataTest, AddAndGetMethod) {
    auto methods = metadata.getMethods("testMethod");
    ASSERT_TRUE(methods.has_value());
    ASSERT_EQ((*methods)->size(), 1);
    
    auto result = (*methods)->front()({}); 
    EXPECT_EQ(result.tryCast<int>(), 42);
}

TEST_F(TypeMetadataTest, AddAndGetProperty) {
    auto property = metadata.getProperty("testProperty");
    ASSERT_TRUE(property.has_value());
    
    BoxedValue obj(TestClass{});
    auto value = property->getter(obj);
    EXPECT_EQ(value.tryCast<int>(), 123);
}

TEST_F(TypeMetadataTest, EventSystem) {
    bool eventFired = false;
    metadata.addEvent("testEvent", "Test event description");
    metadata.addEventListener("testEvent",
        [&eventFired](BoxedValue& obj, const std::vector<BoxedValue>& args) {
            eventFired = true;
        });
        
    BoxedValue obj(TestClass{});
    metadata.fireEvent(obj, "testEvent", {});
    EXPECT_TRUE(eventFired);
}

class TypeRegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        TypeRegistry::instance().registerType("TestClass", TypeMetadata{});
    }
    
    void TearDown() override {
        // Clean up registered types if needed
    }
};

TEST_F(TypeRegistryTest, RegisterAndRetrieveType) {
    auto metadata = TypeRegistry::instance().getMetadata("TestClass");
    ASSERT_TRUE(metadata.has_value());
}

TEST(TypeRegistrarTest, RegisterType) {
    TypeRegistrar<TestClass>::registerType("TestClass");
    
    auto metadata = TypeRegistry::instance().getMetadata("TestClass");
    ASSERT_TRUE(metadata.has_value());
    
    // Test registered constructor
    auto constructor = metadata->getConstructor("TestClass");
    ASSERT_TRUE(constructor.has_value());
    
    auto instance = (*constructor)({});
    ASSERT_TRUE(instance.isType<TestClass>());
}

TEST(HelperFunctionsTest, CallMethod) {
    TypeRegistrar<TestClass>::registerType("TestClass");
    BoxedValue obj(TestClass{});
    
    // Test print method
    EXPECT_NO_THROW(callMethod(obj, "print", {BoxedValue(42)}));
}

TEST(ThreadSafetyTest, ConcurrentTypeRegistration) {
    std::vector<std::thread> threads;
    for(int i = 0; i < 10; i++) {
        threads.emplace_back([i]() {
            std::string typeName = "Type" + std::to_string(i);
            TypeRegistrar<TestClass>::registerType(typeName);
        });
    }
    
    for(auto& thread : threads) {
        thread.join();
    }
    
    // Verify all types were registered
    for(int i = 0; i < 10; i++) {
        std::string typeName = "Type" + std::to_string(i);
        EXPECT_TRUE(TypeRegistry::instance().getMetadata(typeName).has_value());
    }
}

TEST(ErrorHandlingTest, MethodNotFound) {
    TypeRegistrar<TestClass>::registerType("TestClass");
    BoxedValue obj(TestClass{});
    
    EXPECT_THROW(callMethod(obj, "nonexistentMethod", {}), atom::error::NotFound);
}

TEST(EventPriorityTest, EventPriorityOrder) {
    TypeMetadata metadata;
    std::vector<int> executionOrder;
    
    metadata.addEvent("testEvent");
    metadata.addEventListener("testEvent",
        [&](BoxedValue&, const std::vector<BoxedValue>&) {
            executionOrder.push_back(1);
        }, 1);
    metadata.addEventListener("testEvent",
        [&](BoxedValue&, const std::vector<BoxedValue>&) {
            executionOrder.push_back(2);
        }, 2);
        
    BoxedValue obj(TestClass{});
    metadata.fireEvent(obj, "testEvent", {});
    
    ASSERT_EQ(executionOrder.size(), 2);
    EXPECT_EQ(executionOrder[0], 2); // Higher priority executed first
    EXPECT_EQ(executionOrder[1], 1);
}

} // namespace atom::meta::test