#include "addon/loader.hpp"
#include <gtest/gtest.h>

class ModuleLoaderTest : public ::testing::Test {
protected:
    void SetUp() override { loader = lithium::ModuleLoader::createShared(); }

    std::shared_ptr<lithium::ModuleLoader> loader;
};

TEST_F(ModuleLoaderTest, LoadModule) {
    ASSERT_TRUE(loader->loadModule("path/to/module.so", "testModule"));
    ASSERT_TRUE(loader->hasModule("testModule"));
}

TEST_F(ModuleLoaderTest, UnloadModule) {
    ASSERT_TRUE(loader->loadModule("path/to/module.so", "testModule"));
    ASSERT_TRUE(loader->unloadModule("testModule"));
    ASSERT_FALSE(loader->hasModule("testModule"));
}

TEST_F(ModuleLoaderTest, GetModuleInfo) {
    loader->loadModule("path/to/module.so", "testModule");
    auto module = loader->getModule("testModule");
    ASSERT_NE(module, nullptr);
}

TEST_F(ModuleLoaderTest, EnableDisableModule) {
    loader->loadModule("path/to/module.so", "testModule");
    ASSERT_TRUE(loader->enableModule("testModule"));
    ASSERT_TRUE(loader->isModuleEnabled("testModule"));
    ASSERT_TRUE(loader->disableModule("testModule"));
    ASSERT_FALSE(loader->isModuleEnabled("testModule"));
}

TEST_F(ModuleLoaderTest, GetModuleFunctions) {
    loader->loadModule("path/to/module.so", "testModule");
    auto version = loader->getModuleVersion("testModule");
    ASSERT_FALSE(version.empty());
}

TEST_F(ModuleLoaderTest, UnloadAllModules) {
    loader->loadModule("path/to/module.so", "testModule");
    ASSERT_TRUE(loader->unloadAllModules());
    ASSERT_FALSE(loader->hasModule("testModule"));
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
