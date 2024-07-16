#include <gtest/gtest.h>

#include "atom/components/module_macro.hpp"
#include "atom/components/registry.hpp"
#include "atom/error/exception.hpp"

// 初始化和清理函数定义
void init_component_A() {
    // 这个函数留空，只用于测试
}

void cleanup_component_A() {
    // 这个函数留空，只用于测试
}

void init_component_B() {
    // 这个函数留空，只用于测试
}

void cleanup_component_B() {
    // 这个函数留空，只用于测试
}

void init_component_C() {
    // 这个函数留空，只用于测试
}

void cleanup_component_C() {
    // 这个函数留空，只用于测试
}

TEST(RegistryTest, SingletonInstance) {
    Registry& instance1 = Registry::instance();
    Registry& instance2 = Registry::instance();
    ASSERT_EQ(&instance1, &instance2)
        << "Registry should return the same instance for all calls.";
}

TEST(RegistryTest, AddAndInitializeComponents) {
    // 获取Registry实例
    Registry& registry = Registry::instance();

    // 添加组件及其初始化和清理函数
    registry.addInitializer("ComponentA", init_component_A,
                             cleanup_component_A);
    registry.addInitializer("ComponentB", init_component_B,
                             cleanup_component_B);
    registry.addInitializer("ComponentC", init_component_C,
                             cleanup_component_C);

    // 添加依赖关系
    registry.addDependency("ComponentC", "ComponentA");
    registry.addDependency("ComponentC", "ComponentB");

    // 初始化所有组件
    registry.initializeAll();

    // 检查组件是否已初始化
    EXPECT_TRUE(registry.isInitialized("ComponentA"));
    EXPECT_TRUE(registry.isInitialized("ComponentB"));
    EXPECT_TRUE(registry.isInitialized("ComponentC"));

    // 清理所有组件
    registry.cleanupAll();

    // 检查组件是否已清理
    EXPECT_FALSE(registry.isInitialized("ComponentA"));
    EXPECT_FALSE(registry.isInitialized("ComponentB"));
    EXPECT_FALSE(registry.isInitialized("ComponentC"));
}

TEST(RegistryTest, ReinitializeComponent) {
    // 获取Registry实例
    Registry& registry = Registry::instance();

    // 初始化组件A
    registry.initializeAll();

    // 检查组件A是否已初始化
    EXPECT_TRUE(registry.isInitialized("ComponentA"));

    // 重新初始化组件A
    registry.reinitializeComponent("ComponentA");

    // 再次检查组件A是否已初始化
    EXPECT_TRUE(registry.isInitialized("ComponentA"));

    // 清理所有组件
    registry.cleanupAll();

    // 检查组件A是否已清理
    EXPECT_FALSE(registry.isInitialized("ComponentA"));
}

void SampleInit() { /* 初始化逻辑 */ }
void SampleCleanup() { /* 清理逻辑 */ }

TEST(RegistryTest, ComponentInitializationAndCleanup) {
    Registry& registry = Registry::instance();
    registry.addInitializer("SampleComponent", SampleInit, SampleCleanup);

    ASSERT_FALSE(registry.isInitialized("SampleComponent"));
    registry.initializeAll();
    ASSERT_TRUE(registry.isInitialized("SampleComponent"));

    registry.cleanupAll();
    ASSERT_FALSE(registry.isInitialized("SampleComponent"));
}

TEST(RegistryTest, CircularDependency) {
    // 获取Registry实例
    Registry& registry = Registry::instance();

    registry.cleanupAll();

    // 添加循环依赖关系
    registry.addDependency("ComponentA", "ComponentB");
    registry.addDependency("ComponentB", "ComponentA");
    EXPECT_THROW(registry.addDependency("ComponentB", "ComponentA"),
                 atom::error::RuntimeError);

    // 清理所有组件
    registry.cleanupAll();
}

void InitFunc() {}
void CleanupFunc() {}

REGISTER_INITIALIZER(TestComponent, InitFunc, CleanupFunc);
REGISTER_DEPENDENCY(TestComponent, "SampleComponent");

TEST(RegistryTest, MacroBehavior) {
    Registry& registry = Registry::instance();
    registry.initializeAll();  // 应该包含通过宏注册的初始化器
    ASSERT_TRUE(registry.isInitialized("TestComponent"));

    registry.cleanupAll();
    ASSERT_FALSE(registry.isInitialized("TestComponent"));
}

std::vector<std::string> calls;

void initA() { calls.emplace_back("InitA"); }
void cleanupA() { calls.emplace_back("CleanupA"); }
void initB() { calls.emplace_back("InitB"); }
void cleanupB() { calls.emplace_back("CleanupB"); }
void initC() { calls.emplace_back("InitC"); }
void cleanupC() { calls.emplace_back("CleanupC"); }

// 定义宏并注册组件及其依赖
ATOM_EMBED_MODULE(ModuleA, initA);
REGISTER_INITIALIZER(ComponentA, initA, cleanupA);
REGISTER_DEPENDENCY(ComponentA, "ComponentB");

ATOM_EMBED_MODULE(ModuleB, initB);
REGISTER_INITIALIZER(ComponentB, initB, cleanupB);
REGISTER_DEPENDENCY(ComponentB, "ComponentC");

ATOM_EMBED_MODULE(ModuleC, initC);
REGISTER_INITIALIZER(ComponentC, initC, cleanupC);

TEST(RegistryTest, ModuleInitializationAndCleanupOrder) {
    Registry& registry = Registry::instance();

    // 初始化所有组件
    registry.initializeAll();

    // 验证初始化顺序
    ASSERT_EQ(calls.size(), 3);
    EXPECT_EQ(calls[0], "InitC");
    EXPECT_EQ(calls[1], "InitB");
    EXPECT_EQ(calls[2], "InitA");

    calls.clear();  // 清理调用记录，为清理测试做准备

    // 清理所有组件
    registry.cleanupAll();

    // 验证清理顺序
    ASSERT_EQ(calls.size(), 3);
    EXPECT_EQ(calls[0], "CleanupA");
    EXPECT_EQ(calls[1], "CleanupB");
    EXPECT_EQ(calls[2], "CleanupC");
}