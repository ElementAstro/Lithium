#include "atom/components/registry.hpp"
#include "atom/components/macro.hpp"
#include <gtest/gtest.h>
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
    ASSERT_EQ(&instance1, &instance2) << "Registry should return the same instance for all calls.";
}

TEST(RegistryTest, AddAndInitializeComponents) {
    // 获取Registry实例
    Registry& registry = Registry::instance();

    // 添加组件及其初始化和清理函数
    registry.add_initializer("ComponentA", init_component_A,
                             cleanup_component_A);
    registry.add_initializer("ComponentB", init_component_B,
                             cleanup_component_B);
    registry.add_initializer("ComponentC", init_component_C,
                             cleanup_component_C);

    // 添加依赖关系
    registry.add_dependency("ComponentC", "ComponentA");
    registry.add_dependency("ComponentC", "ComponentB");

    // 初始化所有组件
    registry.initialize_all();

    // 检查组件是否已初始化
    EXPECT_TRUE(registry.is_initialized("ComponentA"));
    EXPECT_TRUE(registry.is_initialized("ComponentB"));
    EXPECT_TRUE(registry.is_initialized("ComponentC"));

    // 清理所有组件
    registry.cleanup_all();

    // 检查组件是否已清理
    EXPECT_FALSE(registry.is_initialized("ComponentA"));
    EXPECT_FALSE(registry.is_initialized("ComponentB"));
    EXPECT_FALSE(registry.is_initialized("ComponentC"));
}

TEST(RegistryTest, ReinitializeComponent) {
    // 获取Registry实例
    Registry& registry = Registry::instance();

    // 初始化组件A
    registry.initialize_all();

    // 检查组件A是否已初始化
    EXPECT_TRUE(registry.is_initialized("ComponentA"));

    // 重新初始化组件A
    registry.reinitialize_component("ComponentA");

    // 再次检查组件A是否已初始化
    EXPECT_TRUE(registry.is_initialized("ComponentA"));

    // 清理所有组件
    registry.cleanup_all();

    // 检查组件A是否已清理
    EXPECT_FALSE(registry.is_initialized("ComponentA"));
}

void SampleInit() { /* 初始化逻辑 */ }
void SampleCleanup() { /* 清理逻辑 */ }

TEST(RegistryTest, ComponentInitializationAndCleanup) {
    Registry& registry = Registry::instance();
    registry.add_initializer("SampleComponent", SampleInit, SampleCleanup);
    
    ASSERT_FALSE(registry.is_initialized("SampleComponent"));
    registry.initialize_all();
    ASSERT_TRUE(registry.is_initialized("SampleComponent"));

    registry.cleanup_all();
    ASSERT_FALSE(registry.is_initialized("SampleComponent"));
}

TEST(RegistryTest, CircularDependency) {
    // 获取Registry实例
    Registry& registry = Registry::instance();

    registry.cleanup_all();

    // 添加循环依赖关系
    registry.add_dependency("ComponentA", "ComponentB");
    registry.add_dependency("ComponentB", "ComponentA");
    EXPECT_THROW(registry.add_dependency("ComponentB", "ComponentA"),
                 atom::error::RuntimeError);

    // 清理所有组件
    registry.cleanup_all();
}

void InitFunc() {}
void CleanupFunc() {}

REGISTER_INITIALIZER(TestComponent, InitFunc, CleanupFunc);
REGISTER_DEPENDENCY(TestComponent, "SampleComponent");

TEST(RegistryTest, MacroBehavior) {
    Registry& registry = Registry::instance();
    registry.initialize_all();  // 应该包含通过宏注册的初始化器
    ASSERT_TRUE(registry.is_initialized("TestComponent"));

    registry.cleanup_all();
    ASSERT_FALSE(registry.is_initialized("TestComponent"));
}

std::vector<std::string> calls;

void InitA() { calls.push_back("InitA"); }
void CleanupA() { calls.push_back("CleanupA"); }
void InitB() { calls.push_back("InitB"); }
void CleanupB() { calls.push_back("CleanupB"); }
void InitC() { calls.push_back("InitC"); }
void CleanupC() { calls.push_back("CleanupC"); }

// 定义宏并注册组件及其依赖
ATOM_EMBED_MODULE(ModuleA, InitA);
REGISTER_INITIALIZER(ComponentA, InitA, CleanupA);
REGISTER_DEPENDENCY(ComponentA, "ComponentB");

ATOM_EMBED_MODULE(ModuleB, InitB);
REGISTER_INITIALIZER(ComponentB, InitB, CleanupB);
REGISTER_DEPENDENCY(ComponentB, "ComponentC");

ATOM_EMBED_MODULE(ModuleC, InitC);
REGISTER_INITIALIZER(ComponentC, InitC, CleanupC);

TEST(RegistryTest, ModuleInitializationAndCleanupOrder) {
    Registry& registry = Registry::instance();

    // 初始化所有组件
    registry.initialize_all();

    // 验证初始化顺序
    ASSERT_EQ(calls.size(), 3);
    EXPECT_EQ(calls[0], "InitC");
    EXPECT_EQ(calls[1], "InitB");
    EXPECT_EQ(calls[2], "InitA");

    calls.clear(); // 清理调用记录，为清理测试做准备

    // 清理所有组件
    registry.cleanup_all();

    // 验证清理顺序
    ASSERT_EQ(calls.size(), 3);
    EXPECT_EQ(calls[0], "CleanupA");
    EXPECT_EQ(calls[1], "CleanupB");
    EXPECT_EQ(calls[2], "CleanupC");
}