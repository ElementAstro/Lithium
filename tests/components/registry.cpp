#include "atom/components/registry.hpp"
#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include "components/component.hpp"
#include "error/exception.hpp"

class TestComponent : public Component {
public:
    TestComponent(const std::string& name) : Component(name) {}
    bool initialized = false;
    bool cleaned_up = false;
};

TEST(RegistryTest, AddAndGetComponent) {
    auto& registry = Registry::instance();
    registry.addInitializer("Component1", [](Component&) {}, []() {});
    auto component = registry.getComponent("Component1");
    EXPECT_EQ(component->getName(), "Component1");
}

/*
TEST(RegistryTest, InitializeAndCleanupComponent) {
    auto& registry = Registry::instance();
    registry.cleanupAll();
    auto testComponent = std::make_shared<TestComponent>("Component1");

    registry.addInitializer(
        "Component1", [testComponent]() { testComponent->initialized = true; },
        [testComponent]() { testComponent->cleaned_up = true; });

    registry.initializeAll();
    EXPECT_TRUE(testComponent->initialized);

    registry.cleanupAll();
    EXPECT_TRUE(testComponent->cleaned_up);
}
*/

/*
TEST(RegistryTest, ReinitializeComponent) {
    auto& registry = Registry::instance();
    auto testComponent = std::make_shared<TestComponent>("Component1");

    registry.addInitializer(
        "Component1", [testComponent]() { testComponent->initialized = true; },
        [testComponent]() { testComponent->cleaned_up = true; });

    registry.initializeAll();
    EXPECT_TRUE(testComponent->initialized);

    testComponent->initialized = false;
    registry.reinitializeComponent("Component1");
    EXPECT_TRUE(testComponent->initialized);
}

*/

TEST(RegistryTest, CircularDependencyDetection) {
    auto& registry = Registry::instance();
    registry.addInitializer("Component1", [](Component&) {}, []() {});
    registry.addInitializer("Component2", [](Component&) {}, []() {});

    registry.addDependency("Component1", "Component2");
    EXPECT_THROW(registry.addDependency("Component2", "Component1"),
                 atom::error::RuntimeError);
}

TEST(RegistryTest, DependencyInitializationOrder) {
    auto& registry = Registry::instance();
    registry.cleanupAll();
    std::vector<std::string> initialization_order;

    registry.addInitializer(
        "ComponentA",
        [&](Component&) { initialization_order.emplace_back("ComponentA"); },
        []() {});
    registry.addInitializer(
        "ComponentB",
        [&](Component&) { initialization_order.emplace_back("ComponentB"); },
        []() {});
    registry.addInitializer(
        "ComponentC",
        [&](Component&) { initialization_order.emplace_back("ComponentC"); },
        []() {});

    registry.addDependency("ComponentA", "ComponentB");
    registry.addDependency("ComponentB", "ComponentC");

    registry.initializeAll();

    ASSERT_EQ(initialization_order.size(), 3);
    EXPECT_EQ(initialization_order[0], "ComponentC");
    EXPECT_EQ(initialization_order[1], "ComponentB");
    EXPECT_EQ(initialization_order[2], "ComponentA");
}

/*
TEST(RegistryTest, ComponentInitializationAndCleanup) {
    auto& registry = Registry::instance();
    registry.cleanupAll();
    std::vector<std::string> initialization_order;
    std::vector<std::string> cleanup_order;

    registry.addInitializer(
        "ComponentA",
        [&]() {
            std::cout << "Initializing ComponentA\n";
            initialization_order.push_back("ComponentA");
        },
        [&]() {
            std::cout << "Cleaning up ComponentA\n";
            cleanup_order.push_back("ComponentA");
        });
    registry.addInitializer(
        "ComponentB",
        [&]() {
            std::cout << "Initializing ComponentB\n";
            initialization_order.push_back("ComponentB");
        },
        [&]() {
            std::cout << "Cleaning up ComponentB\n";
            cleanup_order.push_back("ComponentB");
        });
    registry.addInitializer(
        "ComponentC", [&]() { initialization_order.push_back("ComponentC"); },
        [&]() { cleanup_order.push_back("ComponentC"); });

    registry.addDependency("ComponentA", "ComponentB");
    registry.addDependency("ComponentB", "ComponentC");

    registry.initializeAll();
    registry.cleanupAll();

    ASSERT_EQ(initialization_order.size(), 3);
    ASSERT_EQ(cleanup_order.size(), 3);

    EXPECT_EQ(initialization_order[0], "ComponentC");
    EXPECT_EQ(initialization_order[1], "ComponentB");
    EXPECT_EQ(initialization_order[2], "ComponentA");

    EXPECT_EQ(cleanup_order[0], "ComponentA");
    EXPECT_EQ(cleanup_order[1], "ComponentB");
    EXPECT_EQ(cleanup_order[2], "ComponentC");
}
*/
